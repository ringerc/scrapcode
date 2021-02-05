/*
 * Demo program showing VM overcommit on linux.
 *
 * It might be a really bad plan to run this if you're on strict memory
 * allocation (/proc/sys/vm/overcommit_memory = 2).
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>



static void usage_die(const char *argv0, const char *msg);
static void membomb(long int children, long int bytesperchild);

int main(int argc, char * argv[])
{
	if (argc != 3)
		usage_die(argv[0], "wrong argument count");

	char *endptr;

	const long int nchildren = strtol(argv[1], &endptr, 10);
	if (endptr == 0 || *endptr != '\0')
		usage_die(argv[0], "could not parse nchildren");
	if (nchildren <= 0)
		usage_die(argv[0], "nchildren must be > 0");

	const long int bytesperchild = strtol(argv[2], &endptr, 10);
	if (endptr == 0 || *endptr != '\0')
		usage_die(argv[0], "could not parse bytesperchild");
	if (bytesperchild <= 0)
		usage_die(argv[0], "bytesperchild must be > 0");

	FILE * sys_vm_overcommit = fopen("/proc/sys/vm/overcommit_memory", "r");
	char overcommit = fgetc(sys_vm_overcommit);
	fclose(sys_vm_overcommit);
	printf("/proc/sys/vm/overcommit_memory = %c\n", overcommit);

	printf("Will run %ld children to allocate %ld bytes per child\n",
			nchildren, bytesperchild);

	const long int totalbytes = nchildren * bytesperchild;
	printf("This would allocate %ld bytes (%f GiB) of RAM\n",
			totalbytes, (float)totalbytes / (1024*1024*1024));



	membomb(nchildren, bytesperchild);
}

static pid_t fork_child(long int childno, long int bytesperchild)
{
	pid_t child_pid;
	if ((child_pid = fork()) == 0)
	{
		char * unused_mem  = malloc((size_t)bytesperchild);
		/*
		 * Write one byte of the allocation at start and end. This'll
		 * get us a 2 page chunk, 4 KiB, actually budgeted.
		 */
		unused_mem[0] = '\x7f';
		unused_mem[bytesperchild - 1] = '\x7f';

		fprintf(stderr, "child %ld ready\n", childno + 1);
		/* wait for sigterm */
		while (1) {
			sleep(1000);
		}
		/* unreachable in child, exits on sigterm */
	}
	return child_pid;
}

static void membomb(long int children, long int bytesperchild)
{
	pid_t * child_pids = malloc(sizeof(pid_t)*children);

	printf("before launching children:\n");
	system("free -k");
	putchar('\n');

	for (long int childno = 0; childno < children; childno ++)
	{
		pid_t child_pid = fork_child(childno, bytesperchild);
		child_pids[childno] = child_pid;
	}

	/*
	 * can't be bothered reliably waiting to hear from each child
	 * so just wait a few seconds for the "allocations" to finish.
	 */
	sleep(1);

	printf("after launching children:\n");
	system("free -k");
	putchar('\n');

	/*
	 * Report memory use the dumb way. I couldn't be bothered forming a pid
	 * list.
	 */
	char ps_buf[100];
	snprintf(ps_buf, 100, "ps --pid %d --ppid %d -o pid,ppid,pmem,sz,rss,vsz,comm | egrep -v '(ps|sh|grep)'", getpid(), getpid());
	system(ps_buf);

	system("free -h");

	/*
	 * And clean up. Our child procs should exit when we do, but
	 * it doesn't hurt to be tidy.
	 */
	for (long int childno = 0; childno < children; childno++)
		kill(child_pids[childno], SIGTERM);
	for (long int childno = 0; childno < children; childno++)
		waitpid(child_pids[childno], NULL, 0);

	free(child_pids);
}

static void usage_die(const char * argv0, const char * msg)
{
	fprintf(stderr, "error: %s\n", msg);
	fprintf(stderr, "usage: %s nchildren bytesperchild\n", argv0);
	exit(1);
}

/* vm: ts=4 sw=4 noet ai */
