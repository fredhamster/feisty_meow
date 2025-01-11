#!/usr/bin/env bash

# this shell file locates a terminal of the user whose
# login id is specified as the first parameter.

if [ -z "$1" ]; then
  echo $(basename $0): needs a user name as a parameter.
  exit 1
fi

who | grep $1 | awk '{ print $2 }'
