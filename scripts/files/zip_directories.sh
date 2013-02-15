#!/bin/bash

# goes through the current directory (currently) and zips up any directories
# into an archive with the same name as the directory plus a time stamp.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

dirs=()

# snag the command line arguments into an array of names.
for i in "$@"; do dirs+=("$i"); done

# if there were no arguments, use the current directories list of directories.
if [ ${#dirs[@]} -eq 0 ]; then
  # using a temp file is clumsy but after a lot of different methods being tried, this one
  # worked and wasn't super ugly.
  tempfile="$(mktemp /tmp/dirlist.XXXXXX)"
  find $dir -mindepth 1 -maxdepth 1 -type d -exec echo "dirs+=(\"{}\");" ';' >$tempfile
  source "$tempfile"
#echo dirs default to: ${dirs[@]}
  \rm -f $tempfile
fi

# takes a directory name as an argument and sucks the directory
# into a timestamped zip file.
function flattenizer()
{
  for dirname in "$@"; do
    while [[ $dirname =~ .*/$ ]]; do
      dirname="${dirname:0:((${#dirname}-1))}"
    done
    if [ ! -z "$dirname" -a -d "$dirname" ]; then
      echo "flattening '$dirname'..."
      zip -rm "${dirname}_$(date_stringer)" "$dirname" &>/dev/null
    fi
  done
}

for ((i=0; i < ${#dirs[@]}; i++)); do
  dir="${dirs[i]}"
  flattenizer "$dir"
done

