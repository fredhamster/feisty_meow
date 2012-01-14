#!/bin/bash

for i in $*; do
  dirname="$i"

  # make sure this is not a subversion or CVS directory.
  if [[ $i =~ .*\.svn.* || $i =~ .*CVS.* ]]; then
#echo directory matched svn or cvs folder
    continue;
  fi

  # only operate on existent dirs.
  if [ ! -d "$i" ]; then
    continue;
  fi

  entries=($(ls -1 "$i"))
  ent_len=${#entries[*]}
#echo dir $i has entries:
#echo ${entries[*]}
#echo len is $ent_len
  if [ $ent_len -eq 0 ]; then
    # this directory has nothing!
#echo "has zero ents!"
    echo $i
  fi
done

