# Settings common to all runs
FSTYPE ?= xfs
MKFSOPTS=
MOUNTOPTS=

default: standalone-run

.base-build: fsync_test_setup.sh
	docker build -t fsync_error_base -f Dockerfile.base .
	touch .base-build

.standalone-build: .base-build standalone/Dockerfile standalone/fsync-error-clear.c standalone/fsync_test_standalone.sh standalone/Makefile.standalone
	docker build -t fsync_error_standalone -f standalone/Dockerfile .
	touch .standalone-build

# Controls whether we reopen a file between write() and fsync() or
# keep it open.
#
# options are reopen or keepopen; see fsync-error-clear.c
#
REOPEN ?= keepopen

standalone-run: .standalone-build
	docker run --rm --privileged -it fsync_error_test $(FSTYPE) "$(MKFSOPTS)" "$(MOUNTOPTS)" "$(REOPEN)"
