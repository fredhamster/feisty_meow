#!/bin/bash

# a frivolous but useful script that shows information about the local
# computer in terms of an adventure game inventory listing.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

unset -v codename osname osver
if [ $OPERATING_SYSTEM == "UNIX" ]; then
  which lsb_release &>/dev/null
  if [ $? -eq 0 ]; then
    codename="$(lsb_release -cs 2>/dev/null)"
    osname="$(lsb_release -is 2>/dev/null)"
    osver="$(lsb_release -rs 2>/dev/null)"
  fi
fi
if [ -z "$codename" ]; then
  codename="mysterioso"
  osname="unspecified"
  osver="0.0?"
fi

# test if this uptime knows the -p flag.
uptime -p &>/dev/null
if [ $? -eq 0 ]; then
  up="$(uptime -p)"
else
  up="$(uptime | awk '{print $2 " " $3 " " $4 " plus " $1 " hours" }')"
fi

echo
echo "it is $(date +"%A at %H:%M hours on day %e of the %B moon in the gregorian year %Y" | tr A-Z a-z) and our intrepid adventurer $USER is exploring a computer running $osname $osver named $(hostname) (code-name $codename) and has found that the machine's OS platform is $(uname -m) and its current incarnation has been ${up}." | splitter 
#hmmm: splitter not accepting these args properly right now:
#--mincol 2 --maxcol 40
echo
#echo '++++++++++++++++++++++++++++++++++++++++++'
#echo
echo "the following things appear to be lying around here..."
echo
ls -hFC $color_add
echo
#echo '++++++++++++++++++++++++++++++++++++++++++'
#echo
echo "there appear to be these entities on this host..."
echo
who -suT
echo

