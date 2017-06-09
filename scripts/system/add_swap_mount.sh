#!/bin/bash

#hmmm: should check at each step if it worked.
#hmmm: should be able to add a new swap drive if desired.


/bin/dd if=/dev/zero of=/var/swap.1 bs=1M count=1024

/sbin/mkswap /var/swap.1

/sbin/swapon /var/swap.1

free

