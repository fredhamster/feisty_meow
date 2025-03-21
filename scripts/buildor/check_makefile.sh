#!/usr/bin/env bash

for dir in $*; do

  if [ ! -d "$dir" ]; then continue; fi

  problems_seen=
  pushd $dir &>/dev/null
  for i in $(find . -iname "*cpp" -o -iname "*.rc" -o -iname "*.resx" -o -iname "*.cs" ); do

    if [ ! -z "$(echo "$i" | grep '.svn')" ]; then continue; fi

    is_lib_cpp=$(echo $i | sed -n -e 's/^\(.*_library.cpp\)$/\1/p')
    if [ ! -z "$is_lib_cpp" ]; then
      # don't require the X_library.cpp file to be listed.
      continue;
    fi

    # make the exe variant to search for that as a secondary possibility.
    altered_file="$(echo $i | sed -e 's/\.cpp$/\.exe/')"

    for q in makefile*; do
      grep -l $(echo $i | sed -e 's/^\.\///' ) $q &>/dev/null
      if [ $? -ne 0 ]; then

        # try again with the exe form of the name.
        grep -l $(echo "$altered_file" | sed -e 's/^\.\///' ) $q &>/dev/null

        if [ $? -ne 0 ]; then
          # complain now, it's definitely missing.
          echo "missing $i in $dir/$q"
          problems_seen=true
        fi 
      fi 
    done
  done
  popd &>/dev/null

  if [ -z "$problems_seen" ]; then
    echo "makefiles okay in $dir"
  fi

done

