#!/bin/bash
if [ -z "$1" ]; then
  echo You must supply a file name to strip out just the links from...
  exit 2
fi

cat "$1" | sed -n -e 's/^.*[hH][rR][eE][fF]="\([^"]*\)".*$/\1/p'
