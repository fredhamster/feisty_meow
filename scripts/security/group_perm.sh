#!/usr/bin/env bash
# group_perm: traverses directory trees and sets the permissions to allow the
# associated group "rw" on files and "rwx" on directories.

declare -a args=("$@")

if [ -z "${args[*]}" ]; then
  echo "no arguments provided."
  exit 1;
fi

for (( i=0; i < ${#args[*]}; i++ )); do
  current="${args[i]}"
#  echo "curr is $current"
  find -L "$current" -type d -exec chmod u+rwx,g+rwx {} ';'
### >/dev/null 2>/dev/null
  find -L "$current" -type f -exec chmod u+rw,g+rw {} ';'
### >/dev/null 2>/dev/null
done

