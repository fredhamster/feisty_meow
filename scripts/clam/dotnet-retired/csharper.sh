#!/bin/bash
declare -a flipped=()
for i in $*; do
  flip=$(echo $i | sed -e 's/\/\([a-zA-Z]\)\//\1:\//' | tr '/' '\\')
  flipped+=($flip)
done
${FRAMEWORK_DIR}/csc ${flipped[*]}

