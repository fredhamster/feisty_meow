#!/bin/bash

# a simple script for updating a set of folders on a usb stick from subversion or git.  currently
# just runs with no parameters and expects to get all archives from wherever the files originally
# came from.

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
