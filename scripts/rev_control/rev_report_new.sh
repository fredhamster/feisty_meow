#!/bin/bash

# this script reports files that are not checked in yet in a set of folders.
# it works with subversion only, since git handles new files well whereas
# subversion ignores them until you tell it about them.  this script can take
# a directory as a parameter, but will default to the current directory.
# all the directories under the passed directory will be examined.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null

for i in * ; do
  if [ -d "$i" ]; then
    echo "[$i]"
    pushd $i &>/dev/null
    # only update if we see a repository living there.
    if [ -d ".svn" ]; then
      bash $FEISTY_MEOW_SCRIPTS/rev_control/svnapply.sh \? echo
    fi
    popd &>/dev/null
    echo "======="
  fi
done

popd &>/dev/null
