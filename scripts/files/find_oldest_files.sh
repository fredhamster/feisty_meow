#!/bin/bash

# find the most antiquated files in a folder.
# made friendlier for dell by avoiding the .clusterConfig directory and for
# netapp by avoiding the .snapshot directory.

declare -a paths=("${@}")
echo "paths 1 now is: ${paths[@]}"
if [ ${#paths} -eq 0 ]; then
  paths[0]='.'
fi
echo "paths 2 now is: ${paths[@]}"

for pathname in "${paths[@]}"; do

  echo ===============
  echo oldest on path: $pathname
  echo ===============
  echo
 
  find "$pathname" -path "$pathname/.clusterConfig" -prune -o -path "$pathname/.snapshot" -prune -o -type f -print0 | xargs -0 stat --format '%Y :%y %n' | sort -n | cut -d: -f2- | head -n 20

  echo; echo

done

