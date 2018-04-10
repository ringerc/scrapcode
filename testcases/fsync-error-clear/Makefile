# Settings common to all runs
FSTYPE ?= xfs
MKFSOPTS=
MOUNTOPTS=

# Expect PGGIT and PGBRANCH to be set for postgres runs

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
	docker run --rm --privileged -it fsync_error_standalone $(FSTYPE) "$(MKFSOPTS)" "$(MOUNTOPTS)" "$(REOPEN)"

.PHONY: standalone-run




.postgres-build: .base-build postgres/Dockerfile postgres/fsync_test_postgres.sh postgres/pg_fsync_error.py
	docker build -t fsync_error_postgres -f postgres/Dockerfile .
	touch .postgres-build

postgres-run: .postgres-build
	$(if $(PGGIT),,$(error "PGGIT must be defined for postgres-run"))
	$(if $(PGBRANCH),,$(error "PGBRANCH must be defined for postgres-run"))
	docker run --rm --privileged --volume="$(PGGIT)":/postgres --env PGBRANCH="$(PGBRANCH)" -v $(CURDIR)/ccache:/ccache --env CCACHE_DIR=/ccache -it fsync_error_postgres $(FSTYPE) "$(MKFSOPTS)" "$(MOUNTOPTS)" "$(REOPEN)"

.PHONY: postgres-run
