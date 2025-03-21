#!/usr/bin/env bash
# harshperm traverses directory trees and sets the permissions to a restricted
# value (rwx------ for directories and rw------- for files).

declare -a args=("$@")

if [ -z "${args[*]}" ]; then
  echo "no arguments provided."
  exit 1;
fi

for (( i=0; i < ${#args[*]}; i++ )); do
  current="${args[i]}"
#  echo "curr is $current"

  find -L "$current" -type d -exec chmod 700 {} ';'
# >/dev/null 2>/dev/null
  find -L "$current" -type f -exec chmod 600 {} ';'
# >/dev/null 2>/dev/null
done
