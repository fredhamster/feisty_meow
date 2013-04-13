#!/bin/bash
# checks in the updated files in a set of folders checked out from subversion
# or git.  this can take a directory as parameter, but will default to the
# current working directory.  all the directories under the passed directory
# will be examined.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

pushd "$dir" &>/dev/null

for i in * ; do
  if [ -d "$i" ]; then
    echo "[$i]"
    do_checkin $i
#    pushd $i &>/dev/null
#    # only update if we see a repository living there.
#    if [ -d ".svn" ]; then
#      svn ci .
#    elif [ -d ".git" ]; then
#      git commit .
#      git push
#    elif [ -d "CVS" ]; then
#      cvs diff .
#    fi
#    popd &>/dev/null
    echo "======="
  fi
done

popd &>/dev/null


