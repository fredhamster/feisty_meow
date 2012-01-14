#!/bin/bash

# locates any #include declarations inside the files provided as parameters.
# all the includes are turned into their bare file names and sent to standard
# output.

TAB_CHAR=$'\t'

for code_file in $*; do
  echo "$code_file includes:"
  grep -i "^[ $TAB_CHAR]*#include" "$code_file" \
      | sed -e "s/^[ $TAB_CHAR]*#include[ $TAB_CHAR]*[<\"]\(.*\)[>\"].*$/\1/" \
      | tr A-Z a-z \
      | sort | uniq | sort 
done

