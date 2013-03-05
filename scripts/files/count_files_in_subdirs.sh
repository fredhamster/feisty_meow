#!/bin/bash
dirs=($*)
if [ -z "$dirs" ]; then dirs=($(find . -mindepth 1 -maxdepth 1 -type d ) ); fi
#echo dirs are ${dirs[*]}
for i in "${dirs[@]}"; do
  echo -n $(ls -1 $i | wc -l)
  echo -e "\t\t$i"
done

