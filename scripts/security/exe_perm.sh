#!/usr/bin/env bash
# exeperm sets the permissions to an executable value, as for a directory of
# binary files.  (rwxr-xr-x for both directories and files)

declare -a args=("$@")

if [ -z "${args[*]}" ]; then
  echo "no arguments provided."
  exit 1;
fi

for (( i=0; i < ${#args[*]}; i++ )); do
  current="${args[i]}"
#  echo "curr is $current"

  find -L "$current" -type d -exec chmod 755 {} ';'
# >/dev/null 2>/dev/null
  find -L "$current" -type f -exec chmod 755 {} ';'
# >/dev/null 2>/dev/null
done

