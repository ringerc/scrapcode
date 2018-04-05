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
 * rather than the first fsync() as most file systems reserve space before
 * returning from write().
 *
 * Some kernel level info on this behaviour can be found at
 *
 *     http://stackoverflow.com/q/42434872/398670
 *
 * and the PostgreSQL mailing list thread at
 *
 *     https://www.postgresql.org/message-id/CAMsr+YHh+5Oq4xziwwoEfhoTZgr07vdGG+hu=1adXx59aTeaoQ@mail.gmail.com
 */

#define WRITE_BUF_SIZE 4096 * 10

char buf[WRITE_BUF_SIZE];

/*
 * When we've failed a write of our main test file, write a spare second file
 * to see if the FS is still online. This will catch cases where the FS
 * remounts read-only or the like.
 */
static void
write_another_file(const char * base_fname)
{
	int fnamelen = strlen(base_fname) + 5;
	char * fname = malloc(fnamelen);

	fprintf(stderr, "\nTrying to write a second file to test FS\n");

	snprintf(&fname[0], fnamelen, "%s.new", base_fname);
	int fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC);
	if (fd == -1)
	{
		fprintf(stderr, "second file %s: %s\n", fname, strerror(errno));
		exit(1);
	}
	free(fname);

	ssize_t written = write(fd, buf, sizeof(buf));
	if (written == -1)
	{
		perror("second file write()");
		exit(1);
	}
	else if (written != sizeof(buf))
	{
		fprintf(stderr, "second file wrote %zd, expected %zd. Don't know what to do now.\n", written, WRITE_BUF_SIZE);
		exit(1);
	}

	if (fsync(fd) == -1)
	{
		perror("second file fsync()");
		exit(1);
	}

	if (close(fd))
	{
		perror("second file close()");
		exit(1);
	}

	fprintf(stderr, "Wrote second file OK\n");
}

static int
reopen_fd(int fd, const char *fname)
{
	off_t pos = lseek(fd, 0, SEEK_CUR);
	if (pos == (off_t) -1)
	{
		perror("lseek() before close()");
		/* we don't try to continue here */
		exit(1);
	}

	if (close(fd))
	{
		perror("\nerror on close()");
		fprintf(stderr, "Treating error on close() same as write() error; trying reopen and fsync()");
	}

	/*
	 * It seems like nothing particularly interesting here happens unless the
	 * FS does writeback while we don't have the file open. We'll force it with
	 * a sync(). This is, of course, stunningly slow.
	 *
	 * You can instead set dirty_writeback_centisecs and
	 * dirty_expire_centisecs to 1 and usleep(300000) here if you don't
	 * mind waiting a while for the test to run.
	 */
	sync();
	/*usleep(3 * 100 * 1000); */

	fd = open(fname, O_WRONLY);
	if (fd == -1)
	{
		fprintf(stderr, "reopen %s: %s\n", fname, strerror(errno));
		exit(1);
	}

	if (lseek(fd, pos, SEEK_SET) == (off_t) -1)
	{
		perror("lseek() after re-open()");
		exit(1);
	}

	return fd;
}

static void
usage_die(const char *argv0)
{
	fprintf(stderr, "usage: %s path-to-write [reopen|keepopen]\n", argv0);
	exit(2);
}

int main(int argc, char ** argv)
{
	int second_try = 0;
	off_t last_offset = 0;
	int reopen = 0;

	memset(buf, 0, sizeof(buf));

	if (argc < 2)
		usage_die(argv[0]);

	if (argc > 2)
	{
		if (strcmp(argv[2], "reopen") == 0)
			reopen = 1;
		else if (strcmp(argv[2], "keepopen") == 0)
			reopen = 0;
		else
		{
			fprintf(stderr, "unknown open option %s", argv[2]);
			usage_die(argv[0]);
		}
	}

	int fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC);
	if (fd == -1)
	{
		fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
		exit(1);
	}

	fprintf(stderr, "writing %zu byte blocks per fsync() until write() or fsync() error\n",
			WRITE_BUF_SIZE);

	if (reopen)
		fprintf(stderr, "Reopening file between each write() and fsync()\n");
	else
		fprintf(stderr, "Keeping file open across write()s and fsync()s\n");

	/* keep writing until we hit the error */
	int dots = 0;
	while (1)
	{
		int write_err = 0, fsync_err = 0;

		last_offset = lseek(fd, 0, SEEK_CUR);
		if (last_offset == (off_t) -1)
		{
			perror("\nUnexpected error from lseek()");
			exit(3);
		}

		ssize_t written = write(fd, buf, sizeof(buf));
		if (written == -1)
		{
			if (errno == EIO || errno == ENOSPC)
			{
				perror("\nI/O error on write()");
				fprintf(stderr, "File position was %zu before write(), is now %zu",
						last_offset, lseek(fd, 0, SEEK_CUR));
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

		/*
		 * PostgreSQL often write()s to a file, close()s it, re-open()s it, and fsync()s
		 * it in the checkpointer process. We're not bothering to use separate processes
		 * for now, but are simulating the same behaviour of close, write and reopen.
		 *
		 * TODO: We should also test a sync() while closed to test background dirty writeback
		 * handling.
		 */
		if (reopen)
			fd = reopen_fd(fd, argv[1]);

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
				fprintf(stderr, "GOOD: fsync() failed after prior error\n");
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
				fprintf(stderr, "File position was %zu before write(), is now %zu",
						last_offset, lseek(fd, 0, SEEK_CUR));
			}

			if (second_try)
			{
				write_another_file(argv[1]);
				exit(1);
			}

			/*
			 * Go around once more to see what happens on a
			 * subsequent write(), rewinding to retry the failed
			 * write.
			 */
			if (lseek(fd, last_offset, SEEK_SET) == (off_t) -1)
			{
				fprintf(stderr, "lseek() after write() or fsync() error failed with");
				exit(4);
			}

			second_try = 1;
		}

		fprintf(stdout, ".");
		dots++;
		if (dots == 60)
		{
			fprintf(stdout, " %12zu\n", lseek(fd, 0, SEEK_CUR));
			dots = 0;
		}
		fflush(stdout);
	}
}
