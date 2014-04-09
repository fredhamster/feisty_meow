#!/bin/bash

# note that this will only work on the machine with the dhcp server, and
# it only works with our range for unassigned, unknown machines, which is
# from 240 through 249.
logfile="/var/log/messages"
if [ ! -s "$logfile" ]; then
  # a simple fail over that's not guaranteed to work.
  logfile="/var/log/syslog"
fi
tail -n 500 "$logfile" | grep -i "dhcp.*2[4-5][0-9]" | less

