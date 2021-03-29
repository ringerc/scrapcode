/*
 * This demo program helps illustrate Linux memory accounting
 *
 * It allocates various chunks of memory in distinctive sizes, then remains
 * running until killed by a signal. Its only job is to use memory so that tools
 * that examine process and memor state can be tested.
 *
 * If chunk_size_bytes is nonzero, allocates chunks of memory i sizes of powers
 * of 2 on the heap that are variously:
 *
 * - unused (\"heap_untouched\"), 1 chunk
 * - written only on parent process (\"heap_parent\"), 2 chunk
 * - written by parent then child after fork (\"heap_cow\"), 4 chunk
 *
 * If shm_chunk_size_bytes is nonzero, allocates chunks of POSIX shared memory
 * using mmap(..., MAP_SHARED) from a named POSIX shmem segment:
 *
 * - unused, 1 chunk
 * - written only on parent process, 2 chunk
 * - written by parent then by child after fork, 4 chunk
 *
 * By default a child process is fork()ed without a following exec() and used to
 * re-write some of the chunks in order to test accounting for memory touched by
 * multiple processes. Pass 0 to the 3rd argument \"fork_child\" to disable this
 * and use only the parent process.
 *
 * If the 4th argument \"pageout\" is set to 1, madvise(..., MADV_PAGEOUT)
 * will be called on all memory chunks after all have been written by all
 * processes that need to write to them. Requires Linux 4.5 or newer and matching
 * glibc.
 *
 * (The same sort of thing could be done for global arrays in the executable's
 * data sections, but it's not really any different to regular malloc()'d
 * chunks on the heap. Both are CoW memory.)
 *
 * USING CONTROL GROUPS TO LIMIT MEMORY
 * -----
 *
 * To force swapping if MADV_PAGEOUT isn't working, use systemd resource controls.
 * See systemd_run(1) and systemd.resource-control(5) for details. Use systemd-cgls(1) to
 * see process control groups and systemd-cgtop(1) to see resource overviews.
 *
 * For example, to run inside a user-owned "fork_cow_run.scope" cgroup with a
 * memory limit of run (cgroups v2):
 *
 *   systemd-run --no-ask-password --user --scope -u fork_cow_run \
 *     -p MemoryAccounting=1 -p MemoryMax=$((1024*1024*256*4) \
 *     ./fork_cow_demo $((1024*1024*32)) $((1024*1024*256)) 1 1
 *
 * View with
 *
 *   systemd-cgls --user-unit fork_cow_run
 *   systemd-cgtop -m -b -1 --recursive=no user.slice/fork_cow_run.scope
 *   ls /sys/fs/cgroup/unified/system.slice/fork_cow_run.scope/
 *
 * Interesting cgroup fields include memory.usage_in_bytes, memory.memsw.usage_in_bytes, memory.stat, memory.use_hierarchy
 *
 * CHECKING CGROUPS
 * -----
 *
 * To check that memory limits are working, run with a very low MemoryMax /
 * MemoryLimit and make sure you see the output "Killed" when the test program
 * tries to allocate memory. In dmesg you'll see something like
 *
 * 		Memory cgroup out of memory: Killed process 571588 (systemd-run) total-vm:20436kB, anon-rss:996kB, file-rss:8240kB, shmem-rss:0kB, UID:1000 pgtables:80kB oom_score_adj:0
 *		oom_reaper: reaped process 571588 (systemd-run), now anon-rss:0kB, file-rss:0kB, shmem-rss:0kB
 *
 * LEGACY (v1) CGROUPS
 * ------
 *
 * If you have the cgroups v1 legacy mode you'll have to use MemoryLimit
 * instead of MemoryMax, and you won't be able to run in --user mode; see
 * details in systemd resource limits docs. You'll have to remove --no-ask-password
 * too, and use "systemd-cgls -u 'fork_cow_run.scope'" to show the run.
 * Similarly you'll need the group name "system.slice/fork_cow_run.scope" for
 * systemd-cgtop. Info is in /sys/fs/cgroup/memory/system.slice/fork_cow_run.scope/.
 *
 * /proc/$pid/cgroups lists cgroup memberships
 *
 * PROCESS CAPABILITIES
 * -----
 *
 * You can set nondefault capabilities with systemd-run too. This only works
 * with service mode, not scope mode (omit the --scope from systemd-run). Use
 * the -p CapabilityBoundingSet= and -p AmbientCapabilities= options. The unit
 * name will will be system.slice/fork_cow_run.service .
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static void usage_die(const char * argv0, const char * msg)
{
	fprintf(stderr, "error: %s\n", msg);
	fprintf(stderr, "usage: %s chunk_size_bytes [shm_chunk_size_bytes] [fork_child=1|0] [pageout=1|0]\n", argv0);
	fprintf(stderr, "  See program source header for details.\n");

	exit(1);
}

static volatile int got_sigusr1;
static volatile int parent_waiting_for_user;
static void parent_sigint_handler(int sig __attribute__((unused)));
static volatile int parent_stats_update_requested;
static void parent_sighup_handler(int sig __attribute__((unused)));
static void wait_for_sigusr1(void);
static void child_main() __attribute__((noreturn));
static void report_memory_use(pid_t child);
static void * shm_alloc(size_t sz, int shm_fd);
static int setup_shmem(size_t shm_total_size);
static void pageout_chunk(void *addr, size_t length);
static void oom_adjust(pid_t pid);

/*
 * Each chunk we allocate will be 10 MiB multiplied by a power of two.
 * By using powers of two we can determine which memory chunks got
 * charged for where.
 *
 * The same behaviour exists for stack and globals, but it's not
 * worth illustrating that separately.
 */
static size_t chunk_size_bytes, shm_chunk_size_bytes;
static size_t heap_untouched_size, heap_parent_dirty_size, heap_cow_size;
static size_t shm_untouched_size, shm_parent_dirty_size, shm_cow_size;

/* Globals accessible to child after fork() */
static pid_t parent_pid;
static char *heap_cow_dirty_mem;
static char *shm_cow_dirty_mem = 0;
static int pageout = 0;

int main(int argc, char * argv[])
{
	char *endpos;
	int shm_fd = 0;
	int fork_child = 1;
	int wait_signal = 1;

	if (argc < 2 || argc > 6)
		usage_die(argv[0], "wrong argument count");

	if (argc >= 2)
	{
		long int arg_chunk_size_bytes = strtol(argv[1], &endpos, 10);
		if (endpos == 0 || *endpos != '\0')
			usage_die(argv[0], "couldn't parse chunk_size_bytes");
		if (arg_chunk_size_bytes <= 0)
			usage_die(argv[0], "chunk_size must be > 0");

		chunk_size_bytes = (size_t)arg_chunk_size_bytes;

		heap_untouched_size = chunk_size_bytes;
		heap_parent_dirty_size = 2 * chunk_size_bytes;
		heap_cow_size = 4 * chunk_size_bytes;

		const long int total_bytes = heap_untouched_size + heap_parent_dirty_size + heap_cow_size;
		printf("# will allocate %ld private bytes (%f GiB)\n",
				total_bytes, ((float)total_bytes) / (1024*1024*1024));
	}

	if (argc >= 3)
	{
		long int arg_chunk_size_bytes = strtol(argv[2], &endpos, 10);
		if (endpos == 0 || *endpos != '\0')
			usage_die(argv[0], "couldn't parse shm_chunk_size_bytes");
		if (arg_chunk_size_bytes <= 0)
			usage_die(argv[0], "shm_chunk_size must be > 0");

		shm_chunk_size_bytes = (size_t)arg_chunk_size_bytes;

		shm_untouched_size = shm_chunk_size_bytes;
		shm_parent_dirty_size = 2 * shm_chunk_size_bytes;
		shm_cow_size = 4 * shm_chunk_size_bytes;

		const long int total_bytes = shm_untouched_size + shm_parent_dirty_size + shm_cow_size;
		printf("# will allocate %ld shared bytes (%f GiB)\n",
				total_bytes, ((float)total_bytes) / (1024*1024*1024));

		shm_fd = setup_shmem((size_t)total_bytes);
	}

	if (argc >= 4)
	{
		/* Can't be bothered making this check errors */
		fork_child = atoi(argv[3]);
	}

	if (argc >= 5)
	{
		/* Ditto */
		pageout = atoi(argv[4]);
	}

	if (argc >= 6)
	{
		/* Ditto */
		wait_signal = atoi(argv[5]);
	}

	fprintf(stderr, "Configuration: heap_chunk=%ld, shm_chunk=%ld, fork=%d, pageout=%d, wait_signal=%d\n",
			chunk_size_bytes, shm_chunk_size_bytes, fork_child, pageout, wait_signal);

	parent_pid = getpid();

	printf("free -k before parent allocations:\n");
	system("free -k");
	putchar('\n');

	oom_adjust(parent_pid);

	/*
	 * malloc() a big chunk of RAM in the parent process that we won't
	 * actually touch. This memory won't affect our reported memory use
	 * (RSS) at all, though it'll affect our virtual memory size (VSZ).
	 */
	char * heap_untouched_mem __attribute__((unused)) = malloc(heap_untouched_size);
	printf("%20s: %lu\n", "heap_untouched kb", heap_untouched_size / 1024);

	/*
	 * This memory will be fully dirtied in the parent process. The parent
	 * process will get charged for it but the CoW fork()ed child will not.
	 *
	 * In case libc or the kernel decide to do anything clever with optimising
	 * writes of zero to newly allocated memory, write a placeholder value.
	 */
	char *heap_parent_dirty_mem = malloc(heap_parent_dirty_size);
	memset(heap_parent_dirty_mem, '\x7f', heap_parent_dirty_size);
	printf("%20s: %lu\n", "heap_parent_dirty kb", heap_parent_dirty_size/1024);

	/*
	 * This memory will be dirtied by the parent, then again by the child after
	 * fork().
	 *
	 * It should be charged to the child and the parent.
	 */
	heap_cow_dirty_mem = malloc(heap_cow_size);
	memset(heap_cow_dirty_mem, '\x7f', heap_cow_size);
	printf("%20s: %lu\n", "heap_cow kb", heap_cow_size/1024);

	char *shm_untouched_mem __attribute__((unused)) = 0;
	char *shm_parent_dirty_mem = 0;
	if (shm_fd >= 0)
	{
		/* Use the same scheme for shmem allocations */
		shm_untouched_mem = shm_alloc(shm_untouched_size, shm_fd);
		printf("%20s: %lu\n", "shm_untouched kb", shm_untouched_size / 1024);

		shm_parent_dirty_mem = shm_alloc(shm_parent_dirty_size, shm_fd);
		memset(shm_parent_dirty_mem, '\x7f', shm_parent_dirty_size);
		printf("%20s: %lu\n", "shm_parent_dirty kb", shm_parent_dirty_size/1024);

		shm_cow_dirty_mem = shm_alloc(shm_cow_size, shm_fd);
		memset(shm_cow_dirty_mem, '\x7f', shm_cow_size);
		printf("%20s: %lu\n", "shm_cow kb", shm_cow_size/1024);
	}

	long int dirty_private = heap_parent_dirty_size + heap_cow_size;
	long int dirty_shared = shm_parent_dirty_size + shm_cow_size;

	printf("# Parent dirtied %ld bytes (%ld private, %ld shared)\n",
			dirty_private + dirty_shared, dirty_private, dirty_shared);

	putchar('\n');
	printf("free -k after parent allocations, before fork():\n");
	system("free -k");

	/*
	 * fork() a child process without a following exec(). This creates
	 * a copy-on-write shared copy of the parent process's memory in the
	 * child process.
	 */
	pid_t child_pid = 0;

	if (fork_child)
	{
		child_pid = fork();
		if (child_pid == 0) {
			/* In child process. Does not return. */
			child_main();
			assert(0);
		} else {
			/* parent process */
			/*
			 * Child receives same signals as parent process from shell, but
			 * to make sure we don't leak it, tell the kernel to signal it on
			 * our death anyway.
			 */
		}

		/*
		 * Child process will signal us when it has done its memory allocations.
		 */
		wait_for_sigusr1();

		/* Child dirties another chunk of heap memory for the CoW chunks */
		dirty_private += heap_cow_size;
		dirty_shared += shm_cow_size;
		printf("# child dirtied %ld bytes (%ld private, %ld shared)\n",
				heap_cow_size + shm_cow_size, heap_cow_size, shm_cow_size);
	}

	printf("# Parent + child dirtied %ld bytes (%ld private, %ld shared)\n",
			dirty_private + dirty_shared, dirty_private, dirty_shared);

	fflush(stdout);
	fflush(stderr);

	/*
	 * TODO madvise(..., MADV_PAGEOUT) seems to succeed without any results
	 * when run as non-root, only swaps out when run as root. Linux
	 * 5.12.0-0.rc0.20210222git31caf8b2a847.158.vanilla.1.fc33.x86_64
	 * on Fedora 33.
	 */
	if (pageout) {
#ifdef MADV_PAGEOUT
		fprintf(stderr, "\nrequesting MADV_PAGEOUT: ");
		fflush(stderr);
		pageout_chunk(shm_untouched_mem, shm_untouched_size);
		pageout_chunk(shm_parent_dirty_mem, shm_parent_dirty_size);
		pageout_chunk(shm_cow_dirty_mem, shm_cow_size);
		if (shm_fd > 0) {
			pageout_chunk(shm_untouched_mem, shm_untouched_size);
			pageout_chunk(shm_parent_dirty_mem, shm_parent_dirty_size);
			pageout_chunk(shm_cow_dirty_mem, shm_cow_size);
		}
		fprintf(stderr, "done\n");
#else
		fprintf(stderr, "cannot MADV_PAGEOUT: missing libc support\n");
		exit(1);
#endif
	}

	report_memory_use(child_pid);

	if (wait_signal)
	{
		if (isatty(STDIN_FILENO)) {
			/* Handle control-C to break out of loop */
			parent_waiting_for_user = 1;
			signal(SIGINT, parent_sigint_handler);
			while (parent_waiting_for_user) {
				char c = getc(stdin);
				switch (c) {
					case '\r':
					case '\n':
					case ' ':
						fprintf(stderr, "Updated memory info:\n");
						report_memory_use(child_pid);
						break;
					case 'q':
						fprintf(stderr, "Exiting\n");
						parent_waiting_for_user = 0;
						break;
					default:
						fprintf(stderr, "Unknown command char %c. Newline or space to refresh. q or CTRL-C to quit.\n", c);
						break;
				}
			}
			signal(SIGINT, SIG_DFL);
		} else {
			/*
			 * Running as daemon, print info on SIGHUP
			 *
			 * e.g.
			 * 		systemctl kill --signal=SIGHUP fork_cow_run.service
			 */
			signal(SIGHUP, parent_sighup_handler);
			while (1) {
				if (parent_stats_update_requested) {
					report_memory_use(child_pid);
					parent_stats_update_requested = 0;
				}
				sleep(3600);
			}
			signal(SIGHUP, SIG_DFL);
		}
	}

	if (child_pid)
		kill(child_pid, SIGTERM);

	free(heap_untouched_mem);
	free(heap_parent_dirty_mem);
	free(heap_cow_dirty_mem);
	if (shm_fd > 0)
	{
		munmap(shm_untouched_mem, shm_untouched_size);
		munmap(shm_parent_dirty_mem, shm_parent_dirty_size);
		munmap(shm_cow_dirty_mem, shm_cow_size);
	}
}

/*
 * Try to use madvise(.., .., MADV_PAGEOUT) if supported to page out memory.
 */
static void pageout_chunk(void *addr, size_t length)
{
	if (madvise(addr, length, MADV_PAGEOUT) != 0) {
		perror("madvise(%p, %llu, MADV_PAGEOUT): ");
		exit(1);
	}
}

static void print_proc_statm_header(void)
{
	printf("%10s %-10s %10s %10s %10s %10s %10s\n", "pid", "process",
			"size", "resident", "shared", "text", "data");
}

/*
 * See "man 5 proc" for details on /proc/$pid/statm. Note that it
 * reports sizes in 4k pages, not in kb.
 *
 * Ignore the lib and dt fields, and print the rest annotated with the
 * process id and with a header.
 */
static void print_proc_statm(pid_t pid, const char * const label)
{
	static const int KERNEL_PAGE_SIZE_KB = 4;
	char statm_path[40];
	snprintf(statm_path, sizeof(statm_path), "/proc/%u/statm", pid);
	FILE *statm = fopen(statm_path, "r");
	uint64_t statm_size, statm_resident, statm_shared, statm_text, statm_lib, statm_data, statm_dt;
	if (fscanf(statm, "%lu %lu %lu %lu %lu %lu %lu",
				&statm_size, &statm_resident, &statm_shared, &statm_text, &statm_lib, &statm_data, &statm_dt) != 7)
	{
		fprintf(stderr, "fscanf() of %s didn't match all 7 expected fields, giving up\n",
				statm_path);
		exit(1);
	}
	printf("%10d %-10s %10lu %10lu %10lu %10lu %10lu\n",
			pid, label,
			statm_size * KERNEL_PAGE_SIZE_KB,
			statm_resident * KERNEL_PAGE_SIZE_KB,
			statm_shared * KERNEL_PAGE_SIZE_KB,
			statm_text * KERNEL_PAGE_SIZE_KB,
			statm_data* KERNEL_PAGE_SIZE_KB);
	fclose(statm);
}

/*
 * /proc/$pid/smaps_rollup  has sums of process memory allocations.
 *
 * There's also /proc/$pid/smaps, which breaks them down in much more
 * detail, but that's impractical to just print directly here.
 */
static void print_proc_smaps(pid_t pid, const char * const label)
{
	char smaps_rollup_path[40];
	snprintf(smaps_rollup_path, sizeof(smaps_rollup_path), "/proc/%u/smaps_rollup", pid);
	FILE *smaps_rollup = fopen(smaps_rollup_path, "r");
	char * line;
	char linebuf[100];
	(void) fgets(linebuf, sizeof(linebuf), smaps_rollup); /* discard first line */
	while ((line = fgets(linebuf, sizeof(linebuf), smaps_rollup)) != NULL)
		printf("%10d %-10s %s", pid, label, line);
	putchar('\n');
	fclose(smaps_rollup);
}

/*
 * /proc/$pid/status is intended as a more human readable alternative
 * to statm and smaps_rollup.
 *
 * See proc(5)
 */
static void print_proc_status(pid_t pid, const char * const label)
{
	char proc_status_path[40];
	snprintf(proc_status_path, sizeof(proc_status_path), "/proc/%u/status", pid);
	FILE *proc_status = fopen(proc_status_path, "r");
	char * line;
	char linebuf[2048]; /* has long lines and can't be bothered splitting */
	while ((line = fgets(linebuf, sizeof(linebuf), proc_status)) != NULL)
		printf("%10d %-10s %s", pid, label, line);
	putchar('\n');
	fclose(proc_status);
}

static void report_memory_use(pid_t child_pid)
{
	fflush(stdout);
	fflush(stderr);

	putchar('\n');

	/*
	 * ps uses /proc/$pid/stat and /proc/$pid/statm for much of the
	 * information it obtains, though it uses some syscalls too.
	 *
	 * Lets get output from ps first.
	 */
	char ps_cmd_buf[100];
	snprintf(ps_cmd_buf, 100, "ps  -o pid,ppid,pmem,rss,size,vsz,drs,sz -p %d -p %d", parent_pid, child_pid > 0 ? child_pid : parent_pid);
	printf("# ps output:\n");
	system(ps_cmd_buf);

	/*
	 * Now stats reported by the kernel
	 */
	putchar('\n');
	printf("# /proc/$pid/statm info:\n");
	print_proc_statm_header();
	print_proc_statm(parent_pid, "parent");
	if (child_pid > 0)
		print_proc_statm(child_pid, "child");
	putchar('\n');
	printf("# /proc/$pid/smaps_rollup info:\n");
	print_proc_smaps(parent_pid, "parent");
	if (child_pid > 0)
		print_proc_smaps(child_pid, "child");
	printf("# /proc/$pid/status info:\n");
	print_proc_status(parent_pid, "parent");
	if (child_pid > 0)
		print_proc_status(child_pid, "child");

	/*
	 * And system total memory
	 */
	putchar('\n');
	printf("free -k after fork() and CoW overwrite:\n");

	fflush(stdout);
	fflush(stderr);

	system("free -k");
}

/* Break out of the parent wait_signal loop for user input */
static void parent_sigint_handler(int sig __attribute__((unused)))
{
	parent_waiting_for_user = 0;
}

/* Also print stats on SIGHUP */
static void parent_sighup_handler(int sig __attribute__((unused)))
{
	parent_stats_update_requested = 1;
}

static void sigusr1_handler(int sig __attribute__((unused)))
{
	got_sigusr1 = 1;
}

static void wait_for_sigusr1(void)
{
	signal(SIGUSR1, sigusr1_handler);
	got_sigusr1= 0;
	while (!got_sigusr1) {
		/* Will be woken early by signals */
		sleep(1000);
	}
}

 __attribute__((noreturn))
static void child_main(void)
{
	/* See how it's inherited the parent's state? */
	assert(parent_pid == getppid());

	/* Make sure we're signaled if the parent dies */
	prctl(PR_SET_PDEATHSIG, SIGTERM);

	/* Here we'll re-dirty the CoW memory chunk to show we'll get charged for it again */
	memset(heap_cow_dirty_mem, '\x8f', heap_cow_size);

	/* Same for the POSIX shmem segment if shmem was enabled */
	if (shm_cow_dirty_mem)
		memset(shm_cow_dirty_mem, '\x7f', shm_cow_size);

	/* If pageout was requested, try to page out all segments we have touched */
	if (pageout) {
		pageout_chunk(shm_cow_dirty_mem, shm_cow_size);
		if (shm_cow_dirty_mem) {
			pageout_chunk(shm_cow_dirty_mem, shm_cow_size);
		}
	}

	/* Tell parent proc we're ready to be measured */
	kill(parent_pid, SIGUSR1);

	/* Sleep until killed by SIGTERM. The default SIGTERM handler will exit() for us */
	signal(SIGTERM, SIG_DFL);
	while (1) {
		sleep(1000);
	}
	__builtin_unreachable();
}

static void * shm_alloc(size_t sz, int shm_fd)
{
	void * mem = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (mem == 0) {
		fprintf(stderr, "failed to mmap() %ld bytes of posix shmem: %m\n", sz);
		exit(1);
	}
	return mem;
}

/* see "man 7 shm_overview" for info on POSIX shmem */
static int setup_shmem(size_t shm_total_size) {
	int shm_fd;

	/* In case we left a segment from a prior crash */
	shm_unlink("fork_cow_demo");

	/* open the fd in /dev/shmem */
	if ((shm_fd = shm_open("/fork_cow_demo", O_CREAT | O_RDWR | O_EXCL | O_CLOEXEC, 0600)) < 0)
	{
		fprintf(stderr, "failed to shm_open() requested segment: %m\n");
		exit(1);
	}

	/*
	 * Immediately unlink the shmem file. It'll remain accessible to this proc
	 * and its children until they all exit.
	 */
	//.shm_unlink("/fork_cow_demo");

	/* expand the tempfile to the desired size */
	if (ftruncate(shm_fd, shm_total_size) < 0)
	{
		fprintf(stderr, "failed to ftruncate() shmem segment to %ld bytes: %m\n", shm_total_size);
		exit(1);
	}

	return shm_fd;
}

/*
 * Adjust our oom_adj_score to make us attractive OOM killer victims. This
 * protects the rest of the system a bit if the operator does something dumb
 * without a suitable ulimit or cgroup protecting things.
 *
 * Only required on parent, inherited by child if any.
 */
static void oom_adjust(pid_t pid)
{
	char oom_score_adj_path[50];
	snprintf(oom_score_adj_path, sizeof(oom_score_adj_path), "/proc/%u/oom_score_adj", pid);
	FILE *adjfile = fopen(oom_score_adj_path, "w");
	if (!adjfile) {
		fprintf(stderr, "opening %s for write: %m\n", oom_score_adj_path);
		exit(1);
	}
	const char * const score_adj_val = "800\n";
	if (fwrite(score_adj_val, 1, sizeof(score_adj_val), adjfile) != sizeof(score_adj_val)) {
		fprintf(stderr, "writing \"%s\" to %s: %m\n", score_adj_val, oom_score_adj_path);
		exit(1);
	}
	if (fclose(adjfile) != 0) {
		fprintf(stderr, "closing %s after write: %m\n", oom_score_adj_path);
		exit(1);
	}
}

/*
 * vim: ts=4 sw=4 ai noet
 */
