#!/bin/bash

# runs through all the local archives on this host to make sure nothing is
# different when compared with the mainline versions on the specified host.

target="$1"; shift

if [ -z "$target" ]; then
  target=wildmutt
  #target=curie
fi

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function do_a_folder_compare()
{
  local archname="$1"; shift
  local dest="$1"; shift
#hmmm: error checking?

  if [ -d "/z/$archname" ]; then
    sep 14
    echo "Comparing $archname folder..."
    compare_dirs /z/${archname} ${dest}:/z/${archname}
    sep 14
  fi
}

for archicle in \
  basement \
  imaginations \
  musix \
  toaster \
  walrus \
; do
  do_a_folder_compare $archicle $target
done

#cruft:
#sep 14
#echo "Comparing basement folder..."
#compare_dirs /z/basement ${target}:/z/basement
#sep 14
#
#sep 14
#echo "Comparing imaginations folder..."
#compare_dirs /z/imaginations ${target}:/z/imaginations
#sep 14
#
#sep 14
#echo "Comparing musix folder..."
#compare_dirs /z/musix ${target}:/z/musix
#sep 14
#
#sep 14
#echo "Comparing walrus folder..."
#compare_dirs /z/walrus ${target}:/z/walrus
#sep 14
