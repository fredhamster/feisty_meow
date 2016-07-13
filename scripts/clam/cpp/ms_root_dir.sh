#!/bin/bash
# spits out the root directory of visual studio, calculated from the common
# tools directory that always seems to be reliably set.

# code stolen from vis_stu_vars...
if [ -z "$VSxTOOLS" ]; then
  export VSxTOOLS="$VS90COMNTOOLS"
  if [ -z "$VSxTOOLS" ]; then
    export VSxTOOLS="$VS80COMNTOOLS"
  fi
fi
if [ -z "$VSxTOOLS" ]; then
  #echo Failure to locate visual studio tools.
  # don't want to echo anything; need to be able to check that this provided nothing.
  exit 33
fi

short_path "$VSxTOOLS" | sed -e 's/\\/\//g' | sed -e 's/^\(.*\)\/[^\/]*\/[^\/]*[\/]$/\1/'

