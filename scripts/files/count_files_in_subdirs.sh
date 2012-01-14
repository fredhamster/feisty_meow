#!/bin/bash
dirs=$*
if [ -z "$dirs" ]; then dirs=*; fi
for i in $dirs; do
  ls -1 $i | wc | sed -e "s/^ *\([0-9]*\) *.*$/$i: \1/"
done

