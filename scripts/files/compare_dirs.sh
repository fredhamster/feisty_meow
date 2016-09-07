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
#if [ ! -d "$dir1/" -o ! -d "$dir2/" ]; then
#  echo The directories to be compared must already exist.
#  exit 1
#fi
if [ "$dir1" == "$dir2" ]; then
  echo "The two directories are the exact same folder name.  So that's silly."
  exit 1
fi

out1="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"
out2="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"




#hmmm: need error checking here!!!!





if [[ $dir1 == *":"* ]]; then
  # host processing on first dir.
  host1=${dir1%:*}
  dir1=${dir1#*:}
echo "got host1 as $host1"
echo "got new dir1 as $dir1"
fi
if [[ $dir2 == *":"* ]]; then
  # host processing on second dir.
  host2=${dir2%:*}
  dir2=${dir2#*:}
echo "got host2 as $host2"
echo "got new dir2 as $dir2"
fi
if [ -z "$host1" ]; then
  # fully local compare location for first dir.
  pushd "$dir1" &>/dev/null
  find . >"$out1"
  popd &>/dev/null
else
  # remote compare location for first dir.
  ssh "$host1" "cd \"$dir1\" && find ." >"$out1"
fi
# sort the output from that find.
sort "$out1" >"$out1".sort

if [ -z "$host2" ]; then
  # fully local compare location for second dir.
  pushd "$dir2" &>/dev/null
  find . >"$out2"
  popd &>/dev/null
else
  # remote compare location for second dir.
  ssh "$host2" "cd \"$dir2\" && find ." >"$out2"
fi
sort "$out2" >"$out2".sort

diff "$out1".sort "$out2".sort

rm "$out1" "$out1".sort "$out2" "$out2".sort

