#!/usr/bin/env bash

# this script looks for any files containing offensive language, but one must
# fill out the list of inappropriate language to censor.  this list is stored
# in a file in the user's home folder called ".badlanguage".
# given the bad word list (with one obscenity word per line), this will find
# any files that have those words in them and print them out.

OBSCENITY_FILE="$HOME/.badlanguage"

if [ ! -f "$OBSCENITY_FILE" ]; then
  echo "This script requires a file with obscenities at: $OBSCENITY_FILE"
  echo "The file should contain all obscene words that you wish to locate"
  echo "in your files.  Each obscene word should be listed one per line in"
  echo "the bad word file."
  exit 1
fi

# replace the line feeds with a grep pipe.
obscene_line="$(echo $(cat $OBSCENITY_FILE) | sed -e 's/  */\\|/g')" 
#echo "obscenities: $obscene_line"

bash "$FEISTY_MEOW_SCRIPTS/buildor/search_code.sh" "$obscene_line" $*

