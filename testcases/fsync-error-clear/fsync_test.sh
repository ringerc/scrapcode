#!/bin/bash

set -e -u -x

if [ $# -lt 1 ]; then
	echo "usage: $0 xfs|ext3|ext4|vfat|btrfs|... ['mkfs-opts' ['mount-opts']]"
	echo "  e.g. $0 jfs '-q' ''"
	exit 1
fi


FSTYPE=$1

export PATH=/sbin:/usr/sbin:$PATH
if ! which mkfs.$FSTYPE; then
	echo "Cannot find mkfs.$FSTYPE"
	exit 1
fi

MKFSOPTS="${2:-}"

MOUNTOPTS="${3:-}"

REOPEN_MODE="${4:-keepopen}"

mkdir -p /tmp /mnt/tmp
chmod +t /tmp

# cleanup from prior runs
[ "$(dmsetup ls --target=error | cut -f 1)" = "errdev1" ] && dmsetup remove errdev1
for lodev in $(losetup -j /tmp/errblk | cut -d : -f  1)
do
	losetup -d $lodev
done

# make 100mb dev and loopback it
# KB and MB are blocks-to-kb

# Then map a 5k bad-block range near the middle with device-mapper. Some
# twiddling can be needed to put it somewhere the fs doesn't try to write
# something important to. Exactly halfway through the blockdev seems to be
# a problem spot, so don't use that.
#
# producing a fake device with the first 50mb, an error 5k, and the rest of the
# 50mb minus the error range.
BLOCKS_KB=2
BLOCKS_MB=$((1024*2))
# Total looback dev size to allocate
TOTSZ=$((100 * $BLOCKS_MB))
# Amount of loopback device to map before bad range. Move the end of this
# around to make sure the error region doesn't make fs creation fail.
STARTSZ=$(( 58 * $BLOCKS_MB ))
# Amount of loopback device to replace with error region
ERRSZ=$(( 5*$BLOCKS_KB ))
# ... and rest is mapped after

printf "Creating %zu byte block dev with %zu size hole at %zu\n" $(( $TOTSZ * 512 )) $(( $ERRSZ * 512 )) $(( $STARTSZ * 512 ))
echo "Using $FSTYPE"

dd if=/dev/zero of=/tmp/errblk bs=512 count=$TOTSZ

LOOPDEV=$(losetup --show -f /tmp/errblk)

trap "{ losetup -d ${LOOPDEV}; }" EXIT

#
# Units below are 512b blocks
#
# Some twiddling can be needed
#
 
cat > dm_table_errdev1 <<__END__
0				${STARTSZ}				linear ${LOOPDEV} 0
${STARTSZ}			${ERRSZ}				error
$(( $STARTSZ + $ERRSZ ))	$(( $TOTSZ - $STARTSZ - $ERRSZ))	linear ${LOOPDEV} $(( $STARTSZ + $ERRSZ ))
__END__
# (credit http://serverfault.com/q/498900/102814 for approach).

echo "errdev1 table is:"
echo "----"
cat dm_table_errdev1
echo "----"

dmsetup create errdev1 < dm_table_errdev1

trap "{ dmsetup remove errdev1; losetup -d ${LOOPDEV}; }" EXIT

dmsetup mknodes errdev1

sudo mkfs.$FSTYPE $MKFSOPTS /dev/mapper/errdev1

# make a FS on it and mount it
mkdir -p /mnt/tmp
if [ -z "$MOUNTOPTS" ]; then
	OPTSSTR=""
else
	OPTSSTR="-o $MOUNTOPTS"
fi
mount /dev/mapper/errdev1 /mnt/tmp $OPTSSTR

trap "{ trap EXIT; umount /mnt/tmp; dmsetup remove errdev1; losetup -d ${LOOPDEV}; }" EXIT

# Run the test program
./fsync-error-clear /mnt/tmp/test_file "${REOPEN_MODE}"

losetup -d ${LOOPDEV}
trap EXIT
