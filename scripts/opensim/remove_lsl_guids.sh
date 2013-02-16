#!/bin/bash

# a handy script for fixing file names imported using second inventory.
# the names have a hideous long guid as part of them by default.

if [ $# -lt 1 ]; then
  echo "This script requires one or more file names whose names should be fixed."
  echo "Any GUID junk embedded in the name within brackets will be removed."
  exit 1
fi

while [ $# -gt 0 ]; do
  file="$1"; shift
  newname="$(echo "$file" | sed -e 's/\[[a-z0-9A-Z-]*\]//g' | tr ' ' '_' | tr -d "\~'" | sed -e 's/\([0-9]\)_\./\1./g' )"
  if [ "$file" != "$newname" ]; then
    # we've effected a name change, so let's actually do it.
    echo "moving '$file' => '$newname'  "
    mv "$file" "$newname"
  fi
done

