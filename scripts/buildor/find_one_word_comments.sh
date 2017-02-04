#!/bin/bash
# this finds any one-word commented items, which are often a sign of a formal
# parameter that was commented out because it's unused.  that's a suboptimal
# way to fix the compiler warning for some people, and so this will find any
# occurrences of the "wrong"ish way.
# (one different way that some think is more optimal is to use the formal()
# macro to hide the function definition.)

SOURCES_FOUND_LIST="$(mktemp "$TMP/sources_found.XXXXXX")"

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source $FEISTY_MEOW_SCRIPTS/buildor/seek_all_source.sh $*
  # got a list of source files now.

function examine_file {
  file="$1"
  matches="$(sed -n -e 's/\/\*[a-z_A-Z][a-z_A-Z0-9]*\*\//yo/p' <"$file")"
  if [ ! -z "$matches" ]; then echo "$file"; fi
}

while read line_found; do
  if [ $? != 0 ]; then break; fi
  examine_file "$line_found"
done <"$SOURCES_FOUND_LIST"

rm "$SOURCES_FOUND_LIST"

