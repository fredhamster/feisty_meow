#!/bin/bash

# auto-find the scripts, since we might want to run this as sudo.
export WORKDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
source "$WORKDIR/../core/launch_feisty_meow.sh"

#hmmm: should be able to add a new swap drive if desired.

#hmmm: why all the hard-coded paths below?

/bin/dd if=/dev/zero of=/var/swap.1 bs=1M count=2048
test_or_die "creating swap file"

/bin/chmod 600 /var/swap.1
test_or_die "setting swap file permissions"

/sbin/mkswap /var/swap.1
test_or_die "formatting swap file as swap partition"

/sbin/swapon /var/swap.1
test_or_die "enabling new swap partition"

free

