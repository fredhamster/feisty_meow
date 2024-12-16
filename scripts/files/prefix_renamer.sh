#!/usr/bin/env bash

if [ $# -lt 1 ]; then
  echo $0: requires at least one argument as the file prefix to remove or replace.
  echo If there is a second argument, it will be used as the replacement.  Otherwise,
  echo the replacement string will simply be removed from all matching files in the
  echo current directory.
  exit 1
fi

prefix=$1
new_prefix=$2
for i in $prefix*; do
  mv $i $(echo $i | sed -e "s/$prefix*\(.*\)/$new_prefix\1/" )
done

