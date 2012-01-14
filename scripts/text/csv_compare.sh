#!/bin/bash

# this script takes two CSV files and ensures that they have the same
# contents, regardless of the order.
# if the function believes the two files are the same, then zero is returned.
function compare_csv_files()
{
  export file1=$1; shift
  export file2=$1; shift

  if [ ! -f "$file1" -o ! -f "$file2" ]; then
    echo "One of the files is missing; for this utility to work, both:"
    echo "\t$file1"
    echo "\t$file2"
    echo "need to exist before running this script."
    return 3
  fi

  export temp_name1="$(mktemp "$TMP/zz_file1.XXXXXX")"
  export temp_name2="$(mktemp "$TMP/zz_file2.XXXXXX")"
  export temp_out="$(mktemp "$TMP/zz_differences.XXXXXX")"

  # old code: should not be needed after 5.7.425 or so.
  # right now we strip out realtime fields because of a problem in
  # how they are shown (which is as a time and date).
  #sort <"$file1" | grep -v "REALTIME" >"$temp_name1"
  #sort <"$file2" | grep -v "REALTIME" >"$temp_name2"

  # sort the two files so we can ignore the ordering.
  sort <"$file1" >"$temp_name1"
  sort <"$file2" >"$temp_name2"

  diff "$temp_name1" "$temp_name2" >"$temp_out"

  exitval=0

  if [ -s "$temp_out" ]; then
    echo "Differences seen in file:"
    cat "$temp_out"
    # return a failure exit value.
    exitval=2
  fi

  rm "$temp_name1" "$temp_name2" "$temp_out"

  return $exitval
}

# simple run using above function.
compare_csv_files $*


