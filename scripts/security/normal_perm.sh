#!/bin/bash
# normal_perm: traverses directory trees and sets the permissions to a
# standard accessibility value.  for fred, this is rwxr-xr-x for directories
# and rw-r--r-- for files.

declare -a args=("$@")

if [ -z "${args[*]}" ]; then
  echo "no arguments provided."
  exit 1;
fi

for (( i=0; i < ${#args[*]}; i++ )); do
  current="${args[i]}"
#  echo "curr is $current"
  find "$current" -type d -exec chmod 755 {} ';'
### >/dev/null 2>/dev/null
  find "$current" -type f -exec chmod 644 {} ';'
### >/dev/null 2>/dev/null
done

