#!/bin/bash

# travels down each subdirectory and cleans using make.
function clean_subdirectories()
{
  # keep the variable local to avoid contaminating
  # other runs.
  local smoot87

  # find all the subdirectories.
  for smoot87 in $(find . -mindepth 1 -maxdepth 1 -type f); do
echo name is $i

#    # skip if not a directory.
#    if [ ! -d "$smoot87" ]; then continue; fi
#echo "inside first check of it being dir: $smoot87"

    # make sure there's a makefile there.
    if [ -f "$smoot87/$MAKEFILE_NAME" ]; then
echo "inside barriers, with filename=$smoot87 and all conditions met."
      pushd "$smoot87"
      make --silent NOT_FIRST_MAKE=t -f $MAKEFILE_NAME clean
    else
      echo "Skipping makefile-less directory $smoot87..."
    fi
  done
}

# execute our function on current directory.
clean_subdirectories

