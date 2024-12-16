#!/usr/bin/env bash

##############
#
#  Name   : process manager helper methods
#  Author : Chris Koeritz
#  Rights : Copyright (C) 2015-$now by Author
#
# Purpose:
#
#   Relies on the built-in process management in bash to run a bunch of
# processes in the background, but will limit total number running to a
# maximum count.  There is a demonstration method at the end of the file
# that shows how to use the process management functions.
#
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############

#hmmm: revisions desired someday:
#  + allow number of max processes to be passed in.
#  + 

# number of background processes.
bg_count=0

# maximum number of simultaneous background processes.
max_bg_procs=20

# number of processes to wait for if we hit the maximum.
procs_to_await=3

function start_background_action()
{
  # launch the commands provided as parms in a subshell.
  (for i in "${@}"; do eval "$i" ; done)&
  #echo bg_count pre inc is $bg_count
  ((bg_count++))
  #echo bg_count post inc is $bg_count
}

function take_inventory()
{
  start_background_action \
      'echo "taking inventory..."' \
      'bash $FEISTY_MEOW_SCRIPTS/core/inventory.sh'
}

function nechung()
{
  start_background_action \
      'echo "your nechung oracle pronouncement of the moment..."' \
      '$FEISTY_MEOW_BINARIES/nechung'
}

function sum_dir()
{
  start_background_action \
      'echo "summing directory output coming up..."' \
      'perl $FEISTY_MEOW_SCRIPTS/files/summing_dir.pl'
}

# takes the number of processes to wait for, or just waits for one of them.
function wait_on_backgrounders()
{
  local wait_count="$1"; shift

  target_count=$(($bg_count - $wait_count))
  if (($target_count < 1)); then target_count=1; fi
  echo before waiting, count is $bg_count
  while (($bg_count > $target_count - 1)); do
    # wait for one job, let bash pick which.
    wait -n
    echo bg_count pre dec is $bg_count
    ((bg_count--))
    echo bg_count post dec is $bg_count
  done
  echo "done waiting, background process count is down to $bg_count."
}

#hmmm: the demo app here raises some concerns--how do we know the bg count is right?
#      what if something died and we didn't check it?
#      will the thing that looks at bg count adjust it if there are actually no waiting processes?

# happily launches off different actions as background processes.
launcher_demonstrator()
{
  # run a limited number of loops, since we don't want to do this forever.
  local loops=50
  while ((loops-- > 0)); do
    # pick a thing to do.
    which=$(($RANDOM % 3))
#hmmm: not asynch yet!  make it so!
    case $which in
      0) take_inventory;;
      1) nechung;;
      2) sum_dir;;
    esac

    # we have reached the limit on processes and need to wait for a few, defined by
    # procs_to_await variable at top.
    if (($bg_count > $max_bg_procs - 1)); then
      echo "have reached $max_bg_procs background processes threshold; waiting for $procs_to_await of them to complete."
      wait_on_backgrounders $procs_to_await
    fi

  done
}

launcher_demonstrator;

