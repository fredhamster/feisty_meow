#!/bin/bash

# reports whether a disk is spinning physical media or solid state.
# if no drive is specified, then /dev/sda is the default.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

drive="$1"; shift

# plug in a default drive if none is provided.
if [ -z "$drive" ]; then drive="sda"; fi

# chop off the /dev/ portion of the disk name, if it exists.  also chop off
# any partition numbers, since the script can only check whole drives (where,
# so far at least, all partitions on a drive are the same type).
if [[ "$drive" =~ ^/dev/.*$ ]]; then
  drive="$(echo "$drive" | sed -e 's/^\/dev\///')"
#  echo "after mangle, drive is: '$drive'"
fi

#hmmm: could do the check on multiple drives if weren't so lazy.

# let's make sure that the drive exists...
if [ ! -e "/sys/block/${drive}/queue/rotational" ]; then
  false || exit_on_error "failed to find a record for drive '$drive'"
fi

# the value for the block device's rotational parameter should be 1 for hard
# disks and 0 for SSDs.  apparently the linux kernel has supported this check
# since version 2.6.29.
if [ $(cat /sys/block/${drive}/queue/rotational) -eq 0 ]; then
  echo "drive $drive is a solid state disk."
else
  echo "drive $drive is a spinning physical disk."
fi


