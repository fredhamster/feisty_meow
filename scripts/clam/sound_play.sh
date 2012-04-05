#!/bin/bash

# play the sound files specified.

# locates a process given a search pattern to match in the process list.
# copied from functions.h in yeti to avoid intermixing dependencies.
function psfind {
  mkdir -p "$TEMPORARIES_DIR"
  PID_DUMP="$(mktemp "$TEMPORARIES_DIR/zz_pidlist.XXXXXX")"
  appropriate_pattern='s/^[-a-zA-Z_0-9][-a-zA-Z_0-9]*  *\([0-9][0-9]*\).*$/\1/p'
    # pattern to use for peeling off the process numbers.
  extra_flags=
    # flags to pass to ps if any special ones are needed.
  if [ "$OS" = "Windows_NT" ]; then
    # on win32, there is some weirdness to support msys.
    appropriate_pattern='s/^[   ]*\([0-9][0-9]*\).*$/\1/p'
    extra_flags=-W
  fi
  /bin/ps $extra_flags wuax >$PID_DUMP
  # remove the first line of the file, search for the pattern the
  # user wants to find, and just pluck the process ids out of the
  # results.
  PIDS_SOUGHT=$(cat $PID_DUMP \
    | sed -e '1d' \
    | grep -i "$1" \
    | sed -n -e "$appropriate_pattern")
  if [ ! -z "$PIDS_SOUGHT" ]; then echo "$PIDS_SOUGHT"; fi
  /bin/rm $PID_DUMP
}

if [ $# -lt 1 ]; then
  #echo no sound file specified.
  exit 0;
fi

export PLAYCMD=/usr/bin/play
if [ ! -f "$PLAYCMD" ]; then
  PLAYCMD=echo
fi

if [ ! -z "$(psfind pulseaudio)" ]; then
  # we see artsd running...
  PLAYCMD=aplay
elif [ ! -z "$(psfind artsd)" ]; then
  # we see artsd running...
  PLAYCMD=artsplay
elif [ ! -z "$(psfind esd)" ]; then
  # we see esd running...
  PLAYCMD=esdplay
elif [ ! -z "$WINDIR" ]; then
  # kludge for win32; we provide our own sound player.
  PLAYCMD=$CLAM_BIN/playsound
fi

# play the sounds individually; playsound can handle multiple files, but
# "play" doesn't want to on some systems.
for i in $*; do
  ($PLAYCMD $i &>/dev/null &);
done

exit 0
