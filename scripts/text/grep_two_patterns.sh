#!/bin/bash

folder="$1"; shift
pat1="$1"; shift
pat2="$1"; shift

if [ -z "$pat1" -o -z "$pat2" ]; then
  echo "this script requires a folder and two patterns to search."
  echo "all files in the folder (and sub-folders) that contain both patterns"
  echo "will be displayed."
  exit 1
fi

find "$folder" -type f -exec grep -lZ "$pat1" "{}" ';' | xargs -0 grep -l "$pat2"

