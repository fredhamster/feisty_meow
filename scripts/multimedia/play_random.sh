#!/usr/bin/env bash

# plays a random sound chosen from our sound library.

#hmmm: very fred or feisty meow specific right now.

our_host="$(hostname -f)"

#hmmm: soooo antiquated and clunky!  just use a bash variable match expression to do this.
grunty="$(echo "$our_host" | grep -i gruntose.blurgh)"

if [ ! -z "$grunty" ]; then
  VOXDIR=/z/walrus/media/sounds
#hmmm: generalize the above.

#  FILE_LIST=$(find $VOXDIR -type f)
#echo "file list is $FILE_LIST"
#  LINES=$(find $VOXDIR -type f | wc -l)
#echo "info found is: $LINES"
#  LESS_CHANCE=$(expr $LINES \\* 4)
#echo "less chance is $LESS_CHANCE"
#  TO_PLAY=
#  while [ -z "$TO_PLAY" ]; do
#hmmm: random was used here, but not sure where that came from.
#    TO_PLAY=$(find $VOXDIR -type f | random $LESS_CHANCE | tail -n 1)
#hmmm: how was LESS_CHANCE being used previously?  => who cares?  this was bizarre and awful.

  TO_PLAY="$(find $VOXDIR -type f | shuf | tail -n 1)"
#echo "to play is $TO_PLAY"

#  done

  # now play the file we randomly chose.
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $TO_PLAY

fi

