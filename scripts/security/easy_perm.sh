#!/usr/bin/env bash
# easyperm: traverses directory trees and sets the permissions to a completely
# accessible value (rwxrwxrwx for directories and rw-rw-rw- for files).

declare -a args=("$@")

if [ -z "${args[*]}" ]; then
  echo "no arguments provided."
  exit 1;
fi

for (( i=0; i < ${#args[*]}; i++ )); do
  current="${args[i]}"
#  echo "curr is $current"

  find -L "$current" -type d -exec chmod 777 {} ';'
# >/dev/null 2>/dev/null
  find -L "$current" -type f -exec chmod 666 {} ';'
# >/dev/null 2>/dev/null
done

