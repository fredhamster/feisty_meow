#!/usr/bin/env bash

# finds all files that contain any one of the members of a list of patterns.

function print_instructions()
{
  echo "This script requires a directory as the first parameter and then a list of"
  echo "patterns to find within that directory.  All the files matching a pattern"
  echo "in the list will be opened in an editor."

  echo "for example:"

  echo "  $(basename $0 .sh) $FEISTY_MEOW_APEX hoople.net hoople.org"
  echo "the above will search the directory $FEISTY_MEOW_APEX for all matches to"
  echo "the two patterns 'hoople.org' and 'hoople.net'."

  exit 1
}

# capture the first parameter for the directory and remove it.
search_directory="$1"; shift

if [ -z "$search_directory" -o -z "$2" -o ! -d "$search_directory" ]; then
  print_instructions
fi

function launch_editor_on_matching_files()
{
  pushd "$search_directory"

  local donk

  # iterate over the rest of the parameters as patterns.
  for donk in $*; do

    echo "searching [$search_directory] for string [$donk]"
    edit_list="$(bash $FEISTY_MEOW_SCRIPTS/text/search_text.sh $donk)"
    if [ ! -z "$edit_list" ]; then
      gvim $edit_list 2>&1 | cat 
    fi

#np
#hmmm: why doesn't the np alias work?

  done

  popd
}

# invoke our function to do the real work.
launch_editor_on_matching_files $*

##############

# example run for scanning feisty meow code for old domain names that are
# defunct:
# edit_files_matching $FEISTY_MEOW_APEX hoople.net hoople.org hoople.com yeticode.org yeticode.com yeticode.net cromp.com cromp.org cromp.net gruntose.net gruntose.org koeritz.net


