#!/usr/bin/env bash

file="$1"; shift
if [ -z "$file" -o ! -f "$file" ]; then
  echo This script needs a filename to operate on as a parameter.
  echo The file will be examined and the average line length calculated.
  exit 1
fi

cat "$file" | 
awk ' { thislen=length($0); # printf("lines %-5s len %d total %d\n", numlines, thislen, totlen);
  totlen+=thislen 
  if (thislen != 0) { numlines++ } }
END { printf("average line length: %d (no blank lines) or %d (counting blank lines)\n", totlen/numlines, totlen/NR); } ' 


