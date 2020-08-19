#!/bin/bash
if [ -z "$1" ]; then
  echo "Did not find a parameter to seek out and whack from the process list."
  exit 2
fi

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

SCRIPT_NAME="pszap.sh"

PIDLIST=$(psfind -x $SCRIPT_NAME $1)
#echo "PIDS are $PIDLIST"
if [ -z "$PIDLIST" ]; then
  echo "Your process name was not found in the system."
  exit 1
fi
echo "Here are the processes matching your pattern:"
ps $PIDLIST
echo "Are you sure you want to kill all of these? [y/N] "
read answer
if [ "$answer" = "Y" -o "$answer" = "y" ]; then
  kill -9 $PIDLIST
else
  echo "You chose not to zap the processes."
fi

