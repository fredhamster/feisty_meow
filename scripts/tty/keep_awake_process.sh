#!/bin/bash

# This program is meant to be started by the program keep_awake and has
# the guts that are meant to execute inside of a semi-perpetual loop.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

#hmmm: is there still a process management thingy, referred to below, active in our scripts???
# save the process id for the goodbye program to deal with.
#echo $$ >>$TMP/trash.last_keep_awake_process
#don't let the shutdown guy know who we are; we want to keep running now.

# loop sort of forever.
while true; do
  echo -e "\n\ntrying not to fall asleep at $(date_stringer)\n"
  # magical number of seconds to sleep...
  sleep 64
done
