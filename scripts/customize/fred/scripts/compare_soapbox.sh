#!/bin/bash

# compares the soapbox with the real archive to see if any older stuff might be
# left behind.  if it's got a less than in front, then it's only on the soapbox drive
# now rather than the pc's hard drive.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

for currdir in basement imaginations musix walrus; do
  sep
  echo "comparing $currdir where 'less than' is on the soapbox..."
  compare_dirs /media/fred/soapboxdrive/$currdir /z/$currdir
done

sep
