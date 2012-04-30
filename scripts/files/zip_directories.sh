#!/bin/bash

# goes through the current directory (currently) and zips up any directories
# into an archive with the same name as the directory plus a time stamp.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

#hmmm: take a dir to go to for this.
dir=.

for i in $(find $dir -mindepth 1 -maxdepth 1 -type d) ; do zip -rm "${i}_$(date_stringer)" "$i"; done
