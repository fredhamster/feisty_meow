#!/usr/bin/env bash

path="$1"; shift
if [ -z "$path" ]; then
  # assumes our file hierarchy on /z, which is where most of our
  # synching is done against.
  path="/z/"
fi

find "$path" -follow -iname ".sync-conflict*"
