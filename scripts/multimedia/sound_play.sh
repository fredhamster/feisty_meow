#!/bin/bash

# play the sound files specified.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ $# -lt 1 ]; then
  #echo no sound file specified.
  exit 0;
fi

export BASIC_PLAY_CMD='echo Unknown basic sound player...'

if [ -f "/usr/bin/play" ]; then
#echo we see /usr/bin/play available...
  BASIC_PLAY_CMD=/usr/bin/play
elif [ ! -z "$WINDIR" ]; then
#echo "kludge for win32; we provide our own sound player."
  BASIC_PLAY_CMD=playsound
elif [ ! -z "$(whichable afplay)" ]; then
#echo we see afplay available...
  BASIC_PLAY_CMD=afplay
elif [ ! -z "$(psfind artsd)" ]; then
#echo we see artsd running...
  BASIC_PLAY_CMD=artsplay
elif [ ! -z "$(psfind esd)" ]; then
#echo  we see esd running...
  BASIC_PLAY_CMD=esdplay
elif [ ! -z "$(psfind pulseaudio)" ]; then
#echo we see pulse running...
  BASIC_PLAY_CMD="padsp aplay"
elif [ ! -z "$(whichable pw-cat)" ]; then
  BASIC_PLAY_CMD="pw-cat -p"
else
  echo "I don't know how to play basic sound files for this OS and sound system."
fi

export MP3_PLAY_CMD='echo Unknown mp3 player...'

if [ ! -z "$(whichable mplayer)" ]; then
  MP3_PLAY_CMD=mplayer
else
  echo "I don't know how to play mp3 files for this OS and sound system."
fi

# play the sounds individually; some apps like playsound can handle multiple
# files, but "/usr/bin/play" doesn't want to on some systems.
for filename in $*; do 
  case "$filename" in
    *wav)
    $BASIC_PLAY_CMD $filename >/dev/null 2>&1;
    ;;
    *mp3)
    $MP3_PLAY_CMD $filename >/dev/null 2>&1;
    ;;
    *)
    echo "I don't know the file extension here: $filename"
    ;;
  esac
done

exit 0

