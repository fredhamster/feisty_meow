#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

echo '=============='
echo
echo Your user name is $USER on a computer named $(hostname).
echo Your machine platform is $(uname -m)
echo -n Uptime: 
uptime
if [ $OPERATING_SYSTEM == "UNIX" ]; then
  which lsb_release &>/dev/null
  if [ $? -eq 0 ]; then
    lsb_release -a
  fi
fi
echo The time is $(date_stringer | sed -e 's/_/ /g' | sed -e 's/\([0-9][0-9]\) \([0-9][0-9]\)$/:\1:\2/')
echo
echo '=============='
echo
echo You have the following files here:
ls -FC
echo
echo '=============='
echo
echo You are sharing the machine with:
who
echo
echo '=============='

