#!/usr/bin/env bash

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

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

out1="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"
out2="$(mktemp "$TMP/compare_dirs_output.XXXXXX")"


#hmmm: need error checking in here!!!!


# host processing on first dir.
if [[ $dir1 == *":"* ]]; then
  host1=${dir1%:*}
  dir1=${dir1#*:}
#echo "got host1 as $host1 and new dir1 as $dir1"
fi

# host processing on second dir.
if [[ $dir2 == *":"* ]]; then
  host2=${dir2%:*}
  dir2=${dir2#*:}
#echo "got host2 as $host2 and new dir2 as $dir2"
fi

if [ -z "$host1" ]; then
  # fully local compare location for first dir.
  pushd "$dir1" &>/dev/null
  exit_on_error "compare_dirs: seeking directory $dir1"
  find . >"$out1"
  popd &>/dev/null
else
  # remote compare location for first dir.
  ssh "$host1" "cd \"$dir1\" && find ." >"$out1"
  exit_on_error "compare_dirs: listing remote directory $dir1"
fi

# sort the output from listing the first directory.
sort "$out1" >"$out1".sort

if [ -z "$host2" ]; then
  # fully local compare location for second dir.
  pushd "$dir2" &>/dev/null
  exit_on_error "compare_dirs: seeking directory $dir2"
  find . >"$out2"
  popd &>/dev/null
else
  # remote compare location for second dir.
  ssh "$host2" "cd \"$dir2\" && find ." >"$out2"
  exit_on_error "compare_dirs: listing remote directory $dir2"
fi

# sort the output from listing the second directory.
sort "$out2" >"$out2".sort

# compare the two sorted output files to show the missing files on each side.
diff "$out1".sort "$out2".sort

# clean up our output files.
rm "$out1" "$out1".sort "$out2" "$out2".sort

