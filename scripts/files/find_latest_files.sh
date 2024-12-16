#!/usr/bin/env bash

# find last written files, made friendly for dell by avoiding the .clusterConfig directory.

for pathname in "${@}"; do

  echo ==============
  echo latest on path: $pathname
  echo
 
  find "$pathname" -path "$pathname/.clusterConfig" -prune -o -type f -print0 | xargs -0 stat --format '%Y :%y %n' | sort -nr | cut -d: -f2- | head -n 20

  echo; echo

done

