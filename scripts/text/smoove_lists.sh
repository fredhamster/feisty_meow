#!/bin/bash
#list schmoover?
#smoove_lists source target

#takes the contents one level deep in the directories in source
#and stuffs them into similarly named directory in target.
#the directoy under target is named after the last directory
#component in each source directory name.
#here's an example:
#   smoovelists  ~/notes_pending  ~/list_archive
#would move all the files in the directories under notes_pending into
#similar directories under list_archive.
#so, a file ~/notes_pending/foolio/moopter.txt would get moved over to
#a directory called ~/list_archive/moopter.txt
#
#this is useful for coalescing a subset of some to-do items that are under
#directories in a temporary folder into the main archive of to-dos.
#otherwise one must do some manual labor to collapse the directories into
#the right places.
#it only supports one level deep of directories currently because that's
#the most common usage.

if [ $# -lt 2 ]; then
  echo smoove_lists: this requires a source directory where a bunch of list
  echo directories are located and a target where similarly named directories
  echo exist.  the files in the source will be moved over to the target.
  exit 1
fi

source="$(pushd "$1" &>/dev/null; pwd; popd &>/dev/null)"
target="$(pushd "$2" &>/dev/null; pwd; popd &>/dev/null)"

#echo source is $source
#echo target is $target

# create the same directory names under the target if they aren't already
# present under it.
pushd "$source" &>/dev/null

tempfile="$(mktemp "$TMP/zz_smoove.XXXXXX")"

find . -maxdepth 1 -mindepth 1 -type d  >"$tempfile"
while read found; do
  # make a corresponding directory if there isn't one yet.
  if [ ! -d "$target/$found" ]; then
    mkdir -p "$target/$found"
  fi
  # move all the files out of the source and into the target.
  mv "$found"/* "$target/$found/"
done <"$tempfile"

rm "$tempfile"

# clean out any directories that we managed to move everything out of.
perl "$FEISTY_MEOW_SCRIPTS/files/zapdirs.pl" $source

popd &>/dev/null

