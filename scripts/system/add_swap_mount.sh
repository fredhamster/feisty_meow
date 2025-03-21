#!/usr/bin/env bash

# auto-find the scripts, since we might want to run this as sudo.
export THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"
source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

# optional parameters: the instance number of the swap file (best as a number),
# and the size of the swap file to add.
SWAP_INSTANCE="$1"; shift
SWAP_SIZE="$1"; shift

if [ "$SWAP_INSTANCE" == "--help" ]; then
  echo "\
$(basename $0):
This script will add a swap partition for Linux that uses hard drive space for
swap memory.  This increases the amount of available memory on RAM constrained
systems.  It accepts two parameters: (1) the instance of the swap file, which
should be a small number not already used for a swap partition, and (2) the
size of the swap file in megabytes."
  exit 0
fi

# if the swap instance variable is already set, then we'll use it.
# this allows multiple different swap partitions to be added.
if [ -z "$SWAP_INSTANCE" ]; then
  SWAP_INSTANCE=1
fi

# allow the amount of swap space to be determined from outside the script.
# this is measured in megabytes.
if [ -z "$SWAP_SIZE" ]; then
  SWAP_SIZE=2048
fi

/bin/dd if=/dev/zero of=/var/swap.${SWAP_INSTANCE} bs=1M count=${SWAP_SIZE}
exit_on_error "creating swap file"

/bin/chmod 600 /var/swap.${SWAP_INSTANCE}
exit_on_error "setting swap file permissions"

/sbin/mkswap /var/swap.${SWAP_INSTANCE}
exit_on_error "formatting swap file as swap partition"

/sbin/swapon /var/swap.${SWAP_INSTANCE}
exit_on_error "enabling new swap partition"

free

