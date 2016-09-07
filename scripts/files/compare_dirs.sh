#!/bin/bash

# compares the files and directory names in two different top-level directories
# and prints a report of the differences.

dir1="$1"; shift
dir2="$1"; shift

if [ -z "$dir1" -o -z "$dir2" ]; then
  echo This script needs two directory names for which it will create a
  echo list of differences in the two directory hierarchies.
  exit 1
fi
if [ ! -d "$dir1/" -o ! -d "$dir2/" ]; then
  echo The directories to be compared must already exist.
  exit 1
fi
if [ "$dir1" == "$dir2" ]; then
  echo "The two directories are the exact same folder name.  So that's silly."
  exit 1
fi

out1="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"
out2="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"

pushd "$dir1" &>/dev/null
find . >"$out1"
sort "$out1" >"$out1".sort
popd &>/dev/null

pushd "$dir2" &>/dev/null
find . >"$out2"
sort "$out2" >"$out2".sort
popd &>/dev/null

diff "$out1".sort "$out2".sort

rm "$out1" "$out1".sort "$out2" "$out2".sort

