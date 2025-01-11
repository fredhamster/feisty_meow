#!/usr/bin/env bash

# finds the same file name within a hierarchy, if possible.

name="$1"; shift
search_dir="$1"; shift

if [ -z "$name" -o -z "$search_dir" ]; then
  echo This script needs two parameters, the filename to look for and the
  echo directory hierarchy to look for it in.
  exit 1
fi

# just get the filename or directory name component at the end.
name="$(basename "$name")"

find "$search_dir" -iname "$name"


