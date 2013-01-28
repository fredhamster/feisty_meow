#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/build/seek_all_source.sh"

function strip_file {
  file="$1"
  perl $FEISTY_MEOW_SCRIPTS/text/strip_cr.pl "$file"
}

#echo tempfile is $SOURCES_FOUND_LIST

# this block has io redirected from the file to stdin so we can scan the
# file's contents without affecting stdio elsewhere.
while true; do
  read line_found 
  if [ $? != 0 ]; then break; fi
#echo line found=$line_found
  strip_file "$line_found"
done <"$SOURCES_FOUND_LIST"

#echo the source temp is $SOURCES_FOUND_LIST

# clean up.
rm "$SOURCES_FOUND_LIST"

