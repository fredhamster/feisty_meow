#!/bin/bash
hostfile=/etc/HOSTNAME
if [ ! -f "$hostfile" ]; then
  hostfile=/etc/hostname
fi
if [ ! -f "$hostfile" ]; then
  echo "Could not find the hostfile for this machine."
  exit 3
fi

grunty=$(grep -i gruntose.blurgh <"$hostfile")
if [ ! -z "$grunty" ]; then
  VOXDIR=/z/walrus/media/sounds
#hmmm: generalize the above.
  FILE_LIST=$(find $VOXDIR -type f)
echo "file list is $FILE_LIST"
  LINES=$(find $VOXDIR -type f | wc -l)
#echo "info found is: $LINES"
  LESS_CHANCE=$(expr $LINES \\* 4)
#echo "less chance is $LESS_CHANCE"
  TO_PLAY=
  while [ -z "$TO_PLAY" ]; do
    TO_PLAY=$(find $VOXDIR -type f | random $LESS_CHANCE | tail -1)
#echo "to play is $TO_PLAY"
  done

  # now play the file
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $TO_PLAY

fi

