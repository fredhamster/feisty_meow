#!/bin/bash
# this decides randomly whether to invoke the play_random script or not.

NOISE_IT_UP=$(expr $RANDOM / 91)
  # range is 0 to 360 after the division.
#echo noise lev is $NOISE_IT_UP
if [ $NOISE_IT_UP -gt 108 -a $NOISE_IT_UP -le 128 ]; then
  # we hit our percentage.
  bash $FEISTY_MEOW_SCRIPTS/play_random.sh
fi

