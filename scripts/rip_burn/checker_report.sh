#!/bin/bash

# checker_report: runs the checker utility against all files in a directory,
# in such a way that the file count can be very high without blowing its
# mind, and without any extra headers in the report.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

dirname="$1"; shift
outfile="$1"; shift  # optional parm.

if [ -z "$dirname" ]; then
  echo "This script requires one directory on which to make a checker report."
  echo "Additionally, you can specify an output file as the second parameter."
  exit 1
fi

if [ -z "$outfile" ]; then
  outfile="$HOME/checker_report_$(hostname)_$(date_stringer).txt"
fi

if [ -f "$outfile" ]; then
  rm "$outfile"
fi

temp_file_list="$(mktemp /tmp/file_list_temporary.XXXXXX)"

find "$dirname" -type f >"$temp_file_list"
while read input_text; do
  checker -q -b "$input_text" 2>&1 >>"$outfile"
done <"$temp_file_list"

rm "$temp_file_list"


