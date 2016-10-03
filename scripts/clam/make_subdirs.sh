#!/bin/bash

# travels down each subdirectory and builds using make.
function make_subdirectories()
{
  # keep the variable local to avoid contaminating
  # other runs.
  local burlap51

  # find all the subdirectories.
  for burlap51 in $(find . -mindepth 1 -maxdepth 1 -type d); do
    # make sure there's a makefile there.
    if [ -f "$burlap51/makefile" ]; then
#echo "inside barriers, with filename=$burlap51 and all conditions met."
      pushd "$burlap51" &>/dev/null
      make --silent -I "$CLAM_DIR" NOT_FIRST_MAKE=t 
      popd &>/dev/null
    else
      echo "(skipping directory $burlap51)"
    fi
  done
}

# executes our subdirectory make function.
make_subdirectories $*

