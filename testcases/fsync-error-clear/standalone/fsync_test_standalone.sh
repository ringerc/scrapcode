#!/bin/bash

set -e -u -x

. fsync_test_setup.sh

if [ $# -lt 1 ]; then
	echo "usage: $0 xfs|ext3|ext4|vfat|btrfs|... ['mkfs-opts' ['mount-opts' [reopenmode]]]"
	echo "  e.g. $0 jfs '-q' ''"
	exit 1
fi

fs_setup "$1" "$2" "$3"

REOPEN_MODE="${4:-keepopen}"

# Run the test program
./fsync-error-clear /mnt/tmp/test_file "${REOPEN_MODE}"

fs_cleanup
