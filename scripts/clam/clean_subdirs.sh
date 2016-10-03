#!/bin/bash

# travels down each subdirectory and cleans using make.
function clean_subdirectories()
{
  # keep the variable local to avoid contaminating
  # other runs.
  local smoot87

  # find all the subdirectories.
  for smoot87 in $(find . -mindepth 1 -maxdepth 1 -type d); do
    # make sure there's a makefile there.
    if [ -f "$smoot87/makefile" ]; then
#echo "inside barriers, with filename=$smoot87 and all conditions met."
      pushd "$smoot87"
      make --silent -I "$CLAM_DIR" NOT_FIRST_MAKE=t clean
      popd
    else
      echo "Skipping makefile-less directory $smoot87..."
    fi
  done
}

# execute our function on current directory.
clean_subdirectories

