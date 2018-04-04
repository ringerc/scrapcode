#!/bin/bash

set -e -u

mkdir -p /tmp /mnt/tmp
chmod +t /tmp

# cleanup from prior runs
[ "$(dmsetup ls| cut -f 1)" = "errdev1" ] && dmsetup remove errdev1
for lodev in $(losetup -j /tmp/errblk | cut -d : -f  1)
do
	losetup -d $lodev
done

# make 100mb dev and loopback it
dd if=/dev/zero of=/tmp/errblk bs=512 count=204800

LOOPDEV=$(losetup --show -f /tmp/errblk)

trap "{ losetup -d ${LOOPDEV}; }" EXIT

ls -l /dev
ls -l /dev/mapper

# map a 5k hole in the middle with device-mapper
# producing a fake device with the first 50mb, an error 5k, and the last 50mb
#
# Units below are 512b blocks
#
dmsetup create errdev1 --addnodeoncreate <<__END__
0 102400 linear ${LOOPDEV} 0
102400 10 error
102410 102400 linear ${LOOPDEV} 102400
__END__
# (credit http://serverfault.com/q/498900/102814 for approach).

trap "{ dmsetup remove errdev1; losetup -d ${LOOPDEV}; }" EXIT

dmsetup mknodes errdev1

# make a FS on it and mount it
sudo mkfs.xfs /dev/mapper/errdev1
mkdir -p /mnt/tmp
mount /dev/mapper/errdev1 /mnt/tmp

trap "{ trap EXIT; umount /mnt/tmp; dmsetup remove errdev1; losetup -d ${LOOPDEV}; }" EXIT

# Run the test program
./fsync-error-clear /mnt/tmp/test_file

losetup -d ${LOOPDEV}
trap EXIT
