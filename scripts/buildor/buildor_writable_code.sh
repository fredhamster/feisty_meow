#!/bin/bash

source $FEISTY_MEOW_DIR/bin/seek_all_source.sh

function strip_file {
  file=$1
  perl $FEISTY_MEOW_SCRIPTS/strip_cr.pl $file
}

#echo tempfile is $SOURCES_FOUND_LIST

# this block has io redirected from the file to std in.
while true; do
  read line_found 
  if [ $? != 0 ]; then break; fi
  chmod 644 "$line_found"
done <$SOURCES_FOUND_LIST

rm $SOURCES_FOUND_LIST  # clean up.

