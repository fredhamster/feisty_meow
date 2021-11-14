#!/bin/bash

# play the sound files specified.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ $# -lt 1 ]; then
  #echo no sound file specified.
  exit 0;
fi

export PLAYCMD=/usr/bin/play
if [ ! -f "$PLAYCMD" ]; then
  PLAYCMD='echo Unknown sound player...'
fi

if [ ! -z "$(which afplay)" ]; then
#echo we see afplay available...
  PLAYCMD=afplay
elif [ ! -z "$(psfind artsd)" ]; then
#echo we see artsd running...
  PLAYCMD=artsplay
elif [ ! -z "$(psfind esd)" ]; then
#echo  we see esd running...
  PLAYCMD=esdplay
elif [ ! -z "$(psfind pulseaudio)" ]; then
#echo we see pulse running...
  PLAYCMD="padsp aplay"
elif [ ! -z "$WINDIR" ]; then
#echo kludge for win32; we provide our own sound player.
  PLAYCMD=playsound
else
  echo "I don't know how to play sounds for this OS and sound system."
fi

# play the sounds individually; playsound can handle multiple files, but
# "play" doesn't want to on some systems.
for i in $*; do $PLAYCMD $i >/dev/null 2>&1; done

exit 0

