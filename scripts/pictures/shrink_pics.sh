#!/usr/bin/env bash

# picture shrinker, thanks to dang.

if [ $# -lt 2 ]; then
  echo "This script needs at least 2 parameters: parm 1 is the reduction percentage"
  echo "to use when shrinking pictures, and parm 2 (and 3, 4, etc) is a filename"
  echo "to shrink."
  exit 1
fi

percentage="$1"; shift

while true; do
  picname="$1"; shift
  if [ -z "$picname" ]; then
    break
  fi
  mogrify -resize "$percentage%" "$picname";
done

