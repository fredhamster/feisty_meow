#!/bin/sh

if [ ! -f $1 ]; then
  echo "Usage: $0 "
  echo "This script needs a file to sort and a new name for the file after sorting."
  exit 1
fi

echo "Sorting $1 into $2"

sort -t ' ' -k 4.9,4.12n -k 4.5,4.7M -k 4.2,4.3n -k 4.14,4.15n -k 4.17,4.18n -k 4.20,4.21n $1 > $2


