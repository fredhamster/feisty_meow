#!/bin/bash

# checks in all the folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/rev_control.sh"

# selects the method for check-in based on where we are.
function do_checkin()
{
  local directory="$1"; shift
  if [ -d "CVS" ]; then cvs ci . ;
  elif [ -d ".svn" ]; then svn ci . ;
  elif [ -d ".git" ]; then
    git add .  # snag all new files.  not to everyone's liking.
    git commit .  # tell git about all the files and get a check-in comment.
    git push  # upload the files to the server so others can see them.
  else
    echo unknown repository for $directory...
  fi
}

# checks in all the folders in a specified list.
function checkin_list {
  local list=$*
  for i in $list; do
    # turn repo list back into an array.
    eval "repository_list=( ${REPOSITORY_LIST[*]} )"
    for j in "${repository_list[@]}"; do
      # add in the directory component.
      j="$i/$j"
      if [ ! -d "$j" ]; then continue; fi

      pushd $j &>/dev/null
      echo "checking in '$j'..."
      do_checkin $j
      popd &>/dev/null
    done
  done
}

echo "Committing repositories at: $(date)"

if [ "$OS" != "Windows_NT" ]; then
  # first get individual folders.
  checkin_list $HOME
else
  checkin_list $HOME c:/ d:/ e:/ 
fi

