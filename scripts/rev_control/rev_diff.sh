#!/bin/bash
# does differences on a set of folders checked out from subversion or git.
# this can take a directory as parameter, but will default to the current
# working directory.  all the directories under the passed directory will
# be examined.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

tempfile=$(generate_rev_ctrl_filelist)

perform_action_on_file "$tempfile" do_diff


#pushd "$dir" &>/dev/null
#
#for i in * ; do
#  if [ -d "$i" ]; then
#    echo "[$i]"
#    pushd $i &>/dev/null
#    # only update if we see a repository living there.
#    if [ -d ".svn" ]; then
#      svn diff .
#    elif [ -d ".git" ]; then
#      git diff 
#    elif [ -d "CVS" ]; then
#      cvs diff .
#    fi
#    popd &>/dev/null
#    echo "======="
#  fi
#done
#
#popd &>/dev/null

