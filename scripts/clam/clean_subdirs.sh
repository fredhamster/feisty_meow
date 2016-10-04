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
#echo -e "\n\n[[inside barriers, with filename=$burlap51 and all conditions met]]\n\n"
      pushd "$smoot87" &>/dev/null
      make --silent -I "$CLAM_SCRIPTS" NOT_FIRST_MAKE=t clean
      popd &>/dev/null
    else
      echo "(skipping directory $smoot87)"
    fi
  done
}

# execute our function on current directory.
clean_subdirectories

