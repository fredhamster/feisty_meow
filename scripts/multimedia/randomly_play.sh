#!/usr/bin/env bash

# this decides randomly whether to invoke the play_random script or not.  it is a way to
# make random sounds, but not on every single call.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

NOISE_IT_UP=$(expr $RANDOM / 91)
  # range is 0 to 360 after the division.
echo noise lev is $NOISE_IT_UP
if [ $NOISE_IT_UP -gt 108 -a $NOISE_IT_UP -le 142 ]; then
echo hit our percentage and playing a random sound
  # we hit our percentage.
  play_random
fi

