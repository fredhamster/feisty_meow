#!/bin/bash
# a simple script for updating a set of folders on a usb stick from
# subversion.  currently just runs with no parameters and expects to
# get all archives from wherever the files originally came from.
for i in * ; do
  if [ -d "$i" ]; then
    pushd $i
    if [ -d ".svn" ]; then
      # only update if we see a repository living here.
      svn update .
    fi
    popd
  fi
done
