#!/bin/bash

# a frivolous but useful script that shows information about the local
# computer in terms of an adventure game inventory listing.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

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

# see if uptime even exists.
uptime &>/dev/null
if [ $? -eq 0 ]; then
  # test if this uptime knows the -p flag.
  uptime -p &>/dev/null
  if [ $? -eq 0 ]; then
    up="$(uptime -p)"
  else
    up="$(uptime | awk '{print $2 " " $3 " " $4 " plus " $1 " hours" }')"
  fi
else
  # if we can't do this, then we're not even on windows cygwin.  wth are we?
  up="up a whole $(cat /proc/uptime|awk '{print $1}') seconds, yo"
fi

# decide whether they've got splitter available or not.
if [ -f "$FEISTY_MEOW_BINARIES/splitter" -o -f "$FEISTY_MEOW_BINARIES/splitter.exe" ]; then
  # calculate the number of columsn in the terminal.
  cols=$(get_maxcols)
  splitter="$FEISTY_MEOW_BINARIES/splitter --maxcol $(($cols - 1))"
else
  # not available, so just emit as huge overly long string.
  splitter="cat"
fi
echo
echo "it is $(date +"%A at %H:%M hours on day %e of the %B moon in the gregorian year %Y" | tr A-Z a-z) and our intrepid adventurer $USER is exploring a computer named $(hostname) that is running in a thoughtspace called $osname $osver (code-name $codename), and $USER has deduced that the machine's OS platform is $(uname -m) and its current incarnation has been ${up}." | $splitter 
echo
echo "the following things appear to be lying around here..."
echo
ls -hFC $color_add
echo
echo "there appear to be these entities on this host..."
echo
who -suT
echo

