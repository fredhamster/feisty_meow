#!/usr/bin/env bash

# a break out of the popular replace_pattern_in_file function that
# can work with multiple files.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

pattern="$1"; shift
replacement="$1"; shift

if [ -z "$1" -o -z "$pattern" -o -z "$replacement" ]; then
  echo This script requires a simple pattern to search for in a set of files,
  echo the replacement for the pattern, and a list of files.
  exit 1
fi

# loop across the file names we were given.

while true; do
  patt1="$1"; shift
  if [ -z "$patt1" ]; then 
    break;
  fi

  for currfile in $patt1; do
    replace_pattern_in_file "$currfile" "$pattern" "$replacement"
  done

done

