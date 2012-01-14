#!/bin/bash
if [ -z "$1" ]; then
  echo You must supply a file name to strip out the unique movie or show
  echo names from...
  exit 2
fi
cat "$1" | sed -e 's/^\"\([a-zA-Z0-9][a-zA-Z0-9 ]*\)\",\"\([^\"][^\"]*\)\".*$/\2/' | sort | uniq 


