#!/bin/bash

filename="$1"; shift
function_name="$1"; shift

good_path="$(cygpath -w -s $filename)"

#/exports 
dumpbin /all $good_path | grep -q -i "$function_name"
if [ $? -eq 0 ]; then
  echo "Found $function_name in $filename"
fi



