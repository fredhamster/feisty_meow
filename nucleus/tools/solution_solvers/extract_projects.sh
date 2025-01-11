#!/usr/bin/env bash

# this is a simple script that finds the project files listed in a solution file.

solution_name="$1"; shift
if [ -z "$solution_name" ]; then
  echo This script needs a solution or project file name.  It will locate all the
  echo projects listed in that file.
  exit 3
fi

grep -i proj "$solution_name" | sed -n -e 's/.*"\([^"]*proj\)".*/\1/p' | sed -e 's/.*[\\\/]\([^\\\/]*\)/\1/' | tr A-Z a-z | sort | uniq

