#!/bin/bash

cd $FEISTY_MEOW_DIR
  # change to the yeti root.

cd ..
  # go one level up.

export TEMPO_FILE="$(mktemp "$TMP/zz_temp_yeti.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

export XC='--exclude='

tar -czf $TEMPO_FILE $SCRIPT_SYSTEM/* $XC"*.svn/*" $XC"*CVS/*" $XC"*/whole_YETI.tar.gz" $XC"*/yeti_stat*" $XC"*/custom/*"
  # exclude the file names we never want to see.

# the publishing location for the packing.
export YETI_WEB=/var/www/yeti
if [ ! -d $YETI_WEB ]; then
  mkdir $YETI_WEB
fi
if [ ! -d $YETI_WEB ]; then
  echo The web directory $YETI_WEB cannot be created.
  exit 23
fi

# now move the newest version into its resting place.  this prepares the
# hierarchy for uploading; it should be self-consistent.
mv -v $TEMPO_FILE $YETI_WEB/whole_YETI.tar.gz

