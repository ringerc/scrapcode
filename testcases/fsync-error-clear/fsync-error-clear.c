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
 *    LOOPDEV=$(losetup -f /tmp/errblk)
 *    # map a 5k hole in the middle with device-mapper
 *    # producing a fake device with the first 50mb, an error 5k, and the last 50mb
 *    dmsetup create errdev1 <<__END__
 *    0 102400 linear $(LOOPDEV) 0
 *    102400 10 error
 *    102410 102400 linear $(LOOPDEV) 102400
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

#define WRITE_BUF_SIZE 4096 * 10

char buf[WRITE_BUF_SIZE];

int main(int argc, char ** argv)
{
	memset(buf, 0, sizeof(buf));

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

	fprintf(stderr, "writing %zu byte blocks per fsync() until write() or fsync() error\n",
			WRITE_BUF_SIZE);

	/* keep writing until we hit the error */
	int dots = 0;
	while (1)
	{
		int write_err = 0, fsync_err = 0;

		ssize_t written = write(fd, buf, sizeof(buf));
		if (written == -1)
		{
			if (errno == EIO || errno == ENOSPC)
			{
				perror("\nI/O error on write()");
				fprintf(stderr, "Will try to fsync()\n");
				write_err = 1;
			}
			else
			{
				perror("write");
				exit(1);
			}
		}
		else if (written != sizeof(buf))
		{
			fprintf(stderr, "wrote %zd, expected %zd. Don't know what to do now.\n", written, WRITE_BUF_SIZE);
			exit(1);
		}

		if (!write_err && fsync(fd))
		{
			if (errno == EIO|| errno == ENOSPC)
			{
				perror("\nerror on fsync() after successful write()");
				fprintf(stderr, "Will retry fsync()\n");
				fsync_err = 1;
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
		 *
		 * It seems reasonable for fsync() to succeed here on write()
		 * error because we were told about the error directly. But
		 * succeeding after another fsync() failed is ... surprising.
		 */
		if (write_err || fsync_err)
		{
			if (fsync(fd))
			{
				/* Huh, fsync() failed again? */
				perror("fsync");
				fprintf(stderr, "GOOD: fsync() failed like it should (but not what we expected on Linux)\n");
				exit(1);
			}
			else
			{
				/* We expect the error flag to have been cleared by the last call */
				if (fsync_err)
					fprintf(stderr, "BAD: fsync() succeeded after prior fsync() error\n");
				else if (write_err)
					fprintf(stderr, "OK: fsync() succeeded after prior write() error\n");
				else
					abort();
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
