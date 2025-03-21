#!/usr/bin/env bash

#header here!

# lists the files in this (the current) directory that have different names
# than the files in the directory passed as the first (and only) parameter.

list_dir=$1
if [ -z "$list_dir" ]; then
  echo "list_non_dupes"
  echo ""
  echo "This program requires a single parameter, which is a directory name."
  echo "The files in the current directory will be listed to standard output"
  echo "if a file in the user-specified directory exists with the same name."
  exit 1;
fi

for i in *; do
  if [ ! -f "$list_dir/$i" ]; then
    echo "$i"
  fi
done

