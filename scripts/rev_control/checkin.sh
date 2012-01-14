#!/bin/bash

# checks in all our commonly used folders.
# note: fred specific.

source "$SHELLDIR/rev_control/rev_control.sh"

# selects the method for check-in based on where we are.
function do_checkin()
{
  local directory="$1"; shift
  if [ -d "CVS" ]; then
    # this appears to be cvs.
    pushd "$directory/.." &>/dev/null
    cvs ci "$directory"
    popd &>/dev/null
  elif [ -d ".svn" ]; then
    svn ci .
  elif [ -d ".git" ]; then
    git commit .
    git push
  else
    echo unknown repository for $directory...
  fi
}

function checkin_list {
  local list=$*
  for i in $list; do
    for j in $i/feisty_meow $i/hoople $i/hoople2 $i/quartz $i/web $i/yeti $i/xsede/xsede_tests $i/xsede/code/cak0l/trunk ; do
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

