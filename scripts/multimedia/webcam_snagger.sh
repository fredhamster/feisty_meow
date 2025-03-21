#!/usr/bin/env bash

# the file name to be found on the remote site is expected to be named
# using the prefix and suffix below.
# example: prefix=webcam and suffix=jpg leads to a picture named webcam.jpg
FILE_PREFIX=webcam
FILE_SUFFIX=jpg

# this is the location on the internet (or local network) where the file
# can be found.
#WEBPIX_SITE='http://gruntose.com/'
WEBPIX_SITE='ftp://velma/incoming'

# this points at the directory where the downloaded pictures will be stored.
WEBPIX_DIR=$HOME/pix_webcam
if [ ! -d $WEBPIX_DIR ]; then mkdir $WEBPIX_DIR; fi
# make sure that the directory creation worked.
if [ ! -d $WEBPIX_DIR ]; then 
  echo "The target directory $WEBPIX_DIR cannot be created."
  exit 51;
fi

# the number of seconds to sleep between snapshots of the source file.
SNOOZE_PERIOD=3

# our loop variable.  if you want the numbers that are added to the name to
# start at a different value, then change that here.
index=1

while [ $index -lt 10000 ]; do
  # grab the file and store it to a local location.  
  chewed_index=$index
#hmmm: would be nice to have the numbers prefixed by zeros.
  if [ $chewed_index -lt 1000 ]; then chewed_index=0$chewed_index; fi
  if [ $chewed_index -lt 100 ]; then chewed_index=0$chewed_index; fi
  if [ $chewed_index -lt 10 ]; then chewed_index=0$chewed_index; fi

  wget -i $WEBPIX_SITE/$FILE_PREFIX.$FILE_SUFFIX -o $WEBPIX_DIR/$FILE_PREFIX$chewed_index.$FILE_SUFFIX

  index=$(expr $index + 1)
  sleep $SNOOZE_PERIOD
done

