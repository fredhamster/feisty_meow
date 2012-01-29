#!/bin/bash
# This program is meant to be started by the program keep_awake and has
# the basic guts that are meant to execute inside of a semi-perpetual loop.

# save the process id for the goodbye program to deal with.
#echo $$ >>$TMP/trash.last_keep_awake_process
#don't let the shutdown guy know who we are; we want to keep running now.

# loop sort of forever.
while true; do
# this version is for keeping a modem awake.
#  ping -c 7 www.gruntose.com >/dev/null

  echo "trying not to fall asleep at $(date_stringer)"
  sleep 120
done
