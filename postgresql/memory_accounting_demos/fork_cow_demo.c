/*
 * This demo program helps illustrate Linux memory accounting
 *
 * It allocates chunks of memory on the heap that are
 *
 * - unused ("heap_untouched")
 * - written only on parent process ("heap_parent")
 * - written by parent then child after fork ("heap_cow")
 *
 * The same could be done for global arrays but it's not really
 * any different.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <assert.h>

static volatile int got_sigusr1;
static void wait_for_sigusr1(void);
static void child_main() __attribute__((noreturn));
static void report_memory_use(pid_t child);
static void usage_die(const char * argv0, const char *msg);

/*
 * Each chunk we allocate will be 10 MiB multiplied by a power of two.
 * By using powers of two we can determine which memory chunks got
 * charged for where.
 *
 * The same behaviour exists for stack and globals, but it's not
 * worth illustrating that separately.
 */
static size_t chunk_size_bytes;
static size_t heap_untouched_size, heap_parent_dirty_size, heap_cow_size;

/* Globals accessible to child after fork() */
static pid_t parent_pid;
static char *heap_cow_dirty_mem;

int main(int argc, char * argv[])
{
	if (argc != 2)
		usage_die(argv[0], "wrong argument count");

	char *endpos;
	long int arg_chunk_size_bytes = strtol(argv[1], &endpos, 10);
	if (endpos == 0 || *endpos != '\0')
		usage_die(argv[0], "couldn't parse chunk_size_bytes");
	if (arg_chunk_size_bytes <= 0)
		usage_die(argv[0], "chunk_size must be > 0");

	chunk_size_bytes = (size_t)arg_chunk_size_bytes;
	heap_untouched_size = chunk_size_bytes;
	heap_parent_dirty_size = 2 * chunk_size_bytes;
	heap_cow_size = 4 * chunk_size_bytes;

	parent_pid = getpid();
	printf("# parent pid: %u\n\n", parent_pid);

	const long int total_bytes = heap_untouched_size + heap_parent_dirty_size + heap_cow_size;
	printf("# will allocate %ld bytes (%f GiB)\n",
			total_bytes, ((float)total_bytes) / (1024*1024*1024));

	printf("free -k before parent allocations:\n");
	system("free -k");
	putchar('\n');

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

	putchar('\n');
	printf("free -k after parent allocations, before fork():\n");
	system("free -k");

	/*
	 * fork() a child process without a following exec(). This creates
	 * a copy-on-write shared copy of the parent process's memory in the
	 * child process.
	 */
	pid_t child_pid = fork();
	if (child_pid == 0) {
		/* In child process */
		child_main();
	}

	/*
	 * fork returned nonzero so we're still in the parent process.
	 *
	 * Child process will signal us when it has done its memory allocations.
	 */
	wait_for_sigusr1();

	report_memory_use(child_pid);

	kill(child_pid, SIGTERM);
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

static void report_memory_use(pid_t child_pid)
{
	putchar('\n');

	/*
	 * ps uses /proc/$pid/stat and /proc/$pid/statm for much of the
	 * information it obtains, though it uses some syscalls too.
	 *
	 * Lets get output from ps first.
	 */
	char ps_cmd_buf[100];
	snprintf(ps_cmd_buf, 100, "ps -p %d -p %d -o pid,ppid,pmem,rss,size,vsz,drs,sz", parent_pid, child_pid);
	printf("# ps output:\n");
	system(ps_cmd_buf);

	/*
	 * Now stats reported by the kernel
	 */
	putchar('\n');
	printf("# /proc/$pid/statm info:\n");
	print_proc_statm_header();
	print_proc_statm(parent_pid, "parent");
	print_proc_statm(child_pid, "child");
	putchar('\n');
	print_proc_smaps(parent_pid, "parent");
	print_proc_smaps(child_pid, "child");

	/*
	 * And system total memory
	 */
	putchar('\n');
	printf("free -k after fork() and CoW overwrite:\n");
	system("free -k");
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

	/* Here we'll re-dirty the CoW memory chunk to show we'll get charged for it again */
	memset(heap_cow_dirty_mem, '\x8f', heap_cow_size);

	/* Tell parent proc we're ready to be measured */
	kill(parent_pid, SIGUSR1);

	/* Sleep until killed by SIGTERM. The default SIGTERM handler will exit() for us */
	signal(SIGTERM, SIG_DFL);
	while (1) {
		sleep(1000);
	}
	__builtin_unreachable();
}

static void usage_die(const char * argv0, const char * msg)
{
	fprintf(stderr, "error: %s\n", msg);
	fprintf(stderr, "usage: %s chunk_size_bytes\n", argv0);
	exit(1);
}


/*
 * vim: ts=4 sw=4 ai noet
 */
