#!/bin/bash

# compares the soapbox with the real archive to see if any older stuff might be
# left behind.  if it's got a less than in front, then it's only on the soapbox drive
# now rather than the pc's hard drive.

sep
echo "comparing musix where 'less than' is on the soapbox..."
compare_dirs /media/fred/soapboxdrive/musix /z/musix

sep
echo "comparing imaginations where 'less than' is on the soapbox..."
compare_dirs /media/fred/soapboxdrive/imaginations /z/imaginations

sep
echo "comparing walrus where 'less than' is on the soapbox..."
compare_dirs /media/fred/soapboxdrive/walrus /z/walrus

sep
