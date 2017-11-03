#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

#hmmm: should be able to add a new swap drive if desired.

#hmmm: why all the hard-coded paths below?

/bin/dd if=/dev/zero of=/var/swap.1 bs=1M count=1024
check_result "creating swap file"

/bin/chmod 600 /var/swap.1
check_result "setting swap file permissions"

/sbin/mkswap /var/swap.1
check_result "formatting swap file as swap partition"

/sbin/swapon /var/swap.1
check_result "enabling new swap partition"

free

