#!/bin/bash

# checks in all our commonly used folders.
# note: fred specific.

source "$SHELLDIR/rev_control/rev_control.sh"

use_bare_name=0

# selects the method for check-in based on where we are.
function do_checkin()
{
  directory="$1"; shift

  pushd "$directory/.." &>/dev/null

  # get the right modifier for the directory name.
  compute_modifier "$directory" "in"

  is_svn=1
  checkin_cmd="echo unknown repository for $directory..."

  if [ "$home_system" == "true" ]; then
    checkin_cmd="svn ci ."
#    use_bare_name=1
  fi

  # then we pretty much ignore what we guessed, and just use the
  # appropriate command for what we see inside the directory.
  if [ -d "$directory/CVS" ]; then
    checkin_cmd="cvs ci "
    is_svn=0
    modifier=  # reset the modifier, since we know we have cvs.
#    use_bare_name=0
  elif [ -d "$directory/.svn" ]; then
    checkin_cmd="svn ci ."
#    use_bare_name=1
  fi

#  if [ "$use_bare_name" == "1" ]; then
#    directory=$(basename "$directory")
#  fi

  if [ $is_svn -eq 1 ]; then
    pushd "$directory" &>/dev/null
    $checkin_cmd
    popd &>/dev/null
  else
    $checkin_cmd "$modifier$directory"
  fi
  popd &>/dev/null
}

function checkin_list {
  list=$*
  for i in $list; do
    for j in $i/feisty_meow $i/hoople $i/hoople2 $i/quartz $i/web $i/yeti $i/xsede/xsede_tests $i/xsede/code/cak0l/trunk ; do
      if [ ! -d "$j" ]; then
#echo no directory called $j exists
        continue
      fi

      pushd $i &>/dev/null
      folder=$j
      echo "checking in '$folder'..."
      do_checkin $folder
      popd &>/dev/null
    done
  done
}

if [ "$OS" != "Windows_NT" ]; then
  # first get individual folders.
  checkin_list $HOME
##  # now check in the user's directory, if that is an asset in revision control.
##  pushd $HOME  &>/dev/null
##  cd ..
##  echo "checking in '"$(pwd)"/$USER'..."
##  $checkin_cmd $USER
##  popd &>/dev/null
else
  checkin_list c: c:/home d: d:/home e: e:/home f: f:/home g: g:/home h: h:/home i: i:/home 
fi

