#!/bin/bash

set -e -u -x

. fsync_test_setup.sh

if [ $# -lt 1 ]; then
	echo "usage: $0 xfs|ext3|ext4|vfat|btrfs|... ['mkfs-opts' ['mount-opts']]"
	echo "  e.g. $0 jfs '-q' ''"
	exit 1
fi

fs_setup "$1" "$2" "$3"

# Run the test program
DATADIR=/mnt/tmp/pgdata

export PATH=/usr/lib/postgresql/10/bin:$PATH

mkdir -p $DATADIR
adduser --system --home $DATADIR --no-create-home postgres
chown postgres $DATADIR
chmod 700 $DATADIR

find / -name initdb 2>/dev/null

sudo -u postgres `which initdb` -D $DATADIR -A trust -U postgres

# We need the WAL outside the error-prone datadir. Assume the rootfs
# has enough room.
mv $DATADIR/pg_wal /pg_wal
ln -s /pg_wal $DATADIR/pg_wal

cat >> $DATADIR/postgresql.conf <<__END__
listen_addresses='*'
log_checkpoints = on
__END__

sudo -u postgres `which pg_ctl` -D $DATADIR -w start

# We want to produce a load where the normal backend does the writes, and the
# checkpointer handles the fsyncs. We'll do that by forcing regular checkpoints
# via requests from the normal backend.
sudo -u postgres python3 pg_fsync_error.py

sudo -u postgres `which pg_ctl` -D $DATADIR -w stop -m immediate

fs_cleanup
