#!/usr/bin/env bash
if [ -z "$1" -o -z "$2" ]; then
  echo "This program accepts a file name with CSV formatted movie database"
  echo "information and extracts a particular show's episodes.  You need to"
  echo "provide the file name as the first parameter and the show name as"
  echo "the second parameter."
  exit 2
fi
cat "$1" | grep -i "$2" | sed -n -e 's/^\"\([a-zA-Z0-9][a-zA-Z0-9 ]*\)\",\"\([^\"][^\"]*\)\",\"\([^\"][^\"]*\)\"$/"\2" -- \3  [\1]/p' | grep -i ".*$2.* -- " | sort | uniq


