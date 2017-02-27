#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 * THis is a test program demonstrating that when write() or fsync() return
 * an error state with errno == EIO, that return clears the error flag on the
 * fd, so subsequent fsync() calls return success. It is thus wrong to retry
 * fsync() until it succeeds.
 *
 * To test, set up a block device with a bad range in it using device-mapper's 'error' target, e.g.
 *
 *    # make 100mb dev and loopback it
 *    dd if=/dev/zero of=/tmp/errblk bs=512 count=204800
 *    losetup /dev/loop0 /tmp/errblk 
 *    # map a 5k hole in the middle with device-mapper
 *    # producing a fake device with the first 50mb, an error 5k, and the last 50mb
 *    dmsetup create errdev1 <<'__END__'
 *    0 102400 linear /dev/loop0 0
 *    102400 10 error
 *    102410 102400 linear /dev/loop0 102400
 *    __END__
 *    # (credit http://serverfault.com/q/498900/102814 for approach).
 *    # make a FS on it and mount it
 *    sudo mkfs.xfs /dev/mapper/errdev1
 *    mount /dev/mapp/errdev1 /mnt/tmp
 *
 * Then run this test program.
 *
 * You can also observe this by writing until -ENOSPC on a volume without
 * artificial bad blocks, though this will usually fail during write()
 * rather than the first fsync().
 *
 * Some kernel level info on this behaviour can be found at
 *
 *     http://stackoverflow.com/q/42434872/398670
 */

char buf[4096];

int main(int argc, char ** argv)
{
	memset(buf, 0, 4096);

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s path-to-write\n", argv[0]);
		exit(2);
	}

    int fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC);
    if (fd == -1)
    {
		fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
		exit(1);
    }

	/* keep writing until we hit the error */
	int dots = 0;
	while (1)
	{
		int write_err = 0;

		ssize_t written = write(fd, buf, 4096);
		if (written == -1)
		{
			if (errno == EIO || errno == ENOSPC)
			{
				perror("I/O error on write(), will try to fsync()");
				write_err = 1;
			}
			else
			{
				perror("write");
				exit(1);
			}
		}
		else if (written != 4096)
		{
			fprintf(stderr, "wrote %zd, expected 4096. Don't know what to do now.\n", written);
			exit(1);
		}

		if (!write_err && fsync(fd))
		{
			if (errno == EIO|| errno == ENOSPC)
			{
				perror("error on fsync() after successful write(), will retry fsync()");
				write_err = 1;
			}
			else
			{
				perror("fsync1");
				exit(1);
			}
		}

		/*
		 * We just got EIO or ENOSPC from write() or fsync(); another fsync()
		 * should report success since the error was reported by the last
		 * syscall.
		 */
		if (write_err)
		{
			if (fsync(fd))
			{
				/* Huh, fsync() failed again? */
				perror("fsync2");
				exit(1);
			}
			else
			{
				/* We expect the error flag to have been cleared by the last call */
				fprintf(stderr, "second fsync() succeeded as expected\n");
				exit(1);
			}
		}

		fprintf(stdout, ".");
		dots++;
		if (dots == 80)
		{
			fprintf(stdout, "\n");
			dots = 0;
		}
		fflush(stdout);
	}
}
