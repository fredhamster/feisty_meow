#!/bin/bash
# change_prefix: takes all of the files in the current directory starting in $1
# and modifies their prefix to be $2.

if [ $# -lt 2 ]; then
  echo \
"$(basename $0): requires two parameters--the old prefix to look for in this directory"
  echo "and the new prefix (without dots) to change the prefix to."
  \exit 2
fi
for i in *; do
  if [ ! -z "$(echo $i | sed -n -e "s/^\($1\).*\..*$/\1/p")" ]; then
    extrabit="$(echo $i | sed -n -e "s/^$1\(.*\)\..*$/\1/p")"
    suffix="$(echo $i | sed -n -e "s/^$1.*\.\(.*\)$/\1/p")"
#echo extrabit is $extrabit
#echo suff is $suffix
    mv "$i" "$2$extrabit.$suffix"
  fi
done
