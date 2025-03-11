#!/usr/bin/env bash

# plays a random sound chosen from our sound library.

#hmmm: very fred or feisty meow specific right now.

our_host="$(hostname -f)"

# this will eat the entire hostname, if it's a member of our domain.
grunty="${our_host%%*gruntose.blurgh}"
#hmmm: generalize the above also.  oy.

# now check if we should play in our local host zone.
if [ -z "$grunty" ]; then
  VOXDIR=/z/walrus/media/sounds
#hmmm: generalize the above location.

  TO_PLAY="$(find $VOXDIR -type f | shuf | tail -n 1)"
echo "to play is $TO_PLAY"

  # now play the file we randomly chose.
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $TO_PLAY
fi

