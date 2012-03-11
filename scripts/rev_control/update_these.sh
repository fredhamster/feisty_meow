#!/bin/bash
# a simple script for updating a set of folders on a usb stick from subversion or git.  currently
# just runs with no parameters and expects to get all archives from wherever the files originally
# came from.
for i in * ; do
  if [ -d "$i" ]; then
    pushd $i
    # only update if we see a repository living there.
    if [ -d ".svn" ]; then
      svn update .
    elif [ -d ".git" ]; then
      git pull 
    elif [ -d "CVS" ]; then
      cvs update .
    fi
    popd
  fi
done
