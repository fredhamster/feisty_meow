#!/bin/bash

# goes through the current directory (currently) and zips up any directories
# into an archive with the same name as the directory plus a time stamp.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

#hmmm: take a dir parameter to go to for this.
dir=.

function flattenizer()
{
  while read dirname; do
    if [ ! -z "$dirname" ]; then
      echo "flattening dir name is '$dirname'..."
      zip -rm "${dirname}_$(date_stringer)" "$dirname" &>/dev/null
    fi
  done
}

find $dir -mindepth 1 -maxdepth 1 -type d | flattenizer

