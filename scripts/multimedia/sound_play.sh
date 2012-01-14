#!/bin/bash

# play the sound files specified.

source "$SHELLDIR/core/functions.sh"  # provides psfind.

if [ $# -lt 1 ]; then
  #echo no sound file specified.
  exit 0;
fi

export PLAYCMD=/usr/bin/play
if [ ! -f "$PLAYCMD" ]; then
  PLAYCMD=echo
fi

if [ ! -z "$(psfind artsd)" ]; then
  # we see artsd running...
  PLAYCMD=artsplay
elif [ ! -z "$(psfind esd)" ]; then
  # we see esd running...
  PLAYCMD=esdplay
elif [ ! -z "$WINDIR" ]; then
  # kludge for win32; we provide our own sound player.
  PLAYCMD=playsound
fi

# play the sounds individually; playsound can handle multiple files, but
# "play" doesn't want to on some systems.
for i in $*; do $PLAYCMD $i >/dev/null 2>&1; done

exit 0
