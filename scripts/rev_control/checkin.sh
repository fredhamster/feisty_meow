#!/bin/bash

# checks in all our commonly used folders.
# note: fred specific.

source "$FEISTY_MEOW_SCRIPTS/rev_control/rev_control.sh"

# selects the method for check-in based on where we are.
function do_checkin()
{
  local directory="$1"; shift
  if [ -d "CVS" ]; then
    # this appears to be cvs.
#    pushd "$directory/.." &>/dev/null
    cvs ci .
###"$directory"
#    popd &>/dev/null
  elif [ -d ".svn" ]; then
    svn ci .
  elif [ -d ".git" ]; then
    git commit .
    git push
  else
    echo unknown repository for $directory...
  fi
}

# checks in all the folders in a specified list.
function checkin_list {
  local list=$*
  for i in $list; do
echo whole list is ${REPOSITORY_LIST}
    # turn repo list back into an array.
    eval "repository_list=( ${REPOSITORY_LIST[*]} )"
    for j in "${repository_list[@]}"; do
      # add in the directory component.
      j="$i/$j"
echo here the dir thing is: $j
      if [ ! -d "$j" ]; then continue; fi

      pushd $j &>/dev/null
      echo "checking in '$j'..."
      do_checkin $j
      popd &>/dev/null
    done
  done
}

if [ "$OS" != "Windows_NT" ]; then
  # first get individual folders.
  checkin_list $HOME
else
  checkin_list c: c:/home d: d:/home e: e:/home f: f:/home g: g:/home h: h:/home i: i:/home 
fi

