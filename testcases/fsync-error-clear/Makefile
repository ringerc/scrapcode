FSTYPE ?= xfs
MKFSOPTS=
MOUNTOPTS=

# Controls whether we reopen a file between write() and fsync() or
# keep it open.
#
# options are reopen or keepopen; see fsync-error-clear.c
REOPEN ?= keepopen

default: run

build: Dockerfile fsync-error-clear.c fsync_test.sh Makefile.test
	docker build -t fsync_error_test .

run: build
	docker run --rm --privileged -it fsync_error_test $(FSTYPE) "$(MKFSOPTS)" "$(MOUNTOPTS)" "$(REOPEN)"
