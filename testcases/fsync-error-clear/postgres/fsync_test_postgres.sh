#!/bin/bash

set -e -u -x

. fsync_test_setup.sh

if [ $# -lt 1 ]; then
	echo "usage: $0 xfs|ext3|ext4|vfat|btrfs|... ['mkfs-opts' ['mount-opts']]"
	echo "  e.g. $0 jfs '-q' ''"
	exit 1
fi

fs_setup "$1" "$2" "$3"

DATADIR=/mnt/tmp/pgdata
mkdir -p $DATADIR
adduser --system --home $DATADIR --no-create-home postgres
chown postgres $DATADIR
chmod 700 $DATADIR

mkdir -p /ccache
export CCACHE_DIR=${CCACHE_DIR:-/ccache}

git clone -b "${PGBRANCH}" /postgres /pgbuild/git
gitrev=$(cd /pgbuild/git && git rev-parse --short HEAD)
mkdir -p /pgbuild/build
cd /pgbuild/build
../git/configure --prefix=/pgbuild/pg CC="ccache gcc" CPP="ccache cpp"
make -s clean all install
make -s -C contrib clean all install
echo "Installed PostgreSQL ${PGBRANCH} (${gitrev})"

export PATH="/pgbuild/pg/bin:/usr/lib/postgresql/10/bin:${PATH}"

sudo -u postgres `which initdb` -D $DATADIR -A trust -U postgres

# We need the WAL outside the error-prone datadir. Assume the rootfs
# has enough room.
mv $DATADIR/pg_wal /pg_wal
ln -s /pg_wal $DATADIR/pg_wal

cat >> $DATADIR/postgresql.conf <<__END__
listen_addresses='*'
log_checkpoints = on
log_error_verbosity = verbose
__END__

sudo -u postgres `which pg_ctl` -D $DATADIR -w start

# We want to produce a load where the normal backend does the writes, and the
# checkpointer handles the fsyncs. We'll do that by forcing regular checkpoints
# via requests from the normal backend.
sudo -u postgres PGHOST=/tmp python3 /pg_fsync_error.py

sudo -u postgres `which pg_ctl` -D $DATADIR -w stop -m immediate

pkill postgres
pkill python
sleep 1
pkill -9 postgres
pkill -9 python
fs_cleanup
