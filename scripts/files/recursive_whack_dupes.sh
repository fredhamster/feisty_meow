#!/usr/bin/env bash

###############################################################################
#                                                                             #
#  Name   : recursive_whack_dupes                                             #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2007-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Recursively whacks duplicate files when given the exemplary "good"       #
#  directory as the first parameter and a potentially shadow directory of     #
#  lesser importance.  The second directory will be cleaned out where it has  #
#  filenames that are the same as in the exemplar directory.                  #
#                                                                             #
###############################################################################
#  This script is free software; you can redistribute it and/or modify it     #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy  #
#  of the License online.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

exemplar_dir=$1
whack_dir=$2

if [ -z "$1" -o -z "$2" ]; then
  echo This program requires two parameters.  The first is a directory that is
  echo considered to have a set of good files in it.  The second is a directory
  echo that has possible duplicates of the files in the good directory.  Any
  echo files found in the second directory will be removed if they are already
  echo in the first directory.  This will be done recursively so that any
  echo directory names in the exemplar that are shadowed by the whacking directory
  echo will have their duplicate files cleaned out too.  The result will be that
  echo the whacking directory (second parameter) will have just the folders and
  echo non-duplicate filenames remaining and the exemplar directory (first parameter)
  echo will be untouched.
  exit 1
fi

# change to the whacking arena.
pushd $whack_dir &>/dev/null

for i in $(find . -depth -mindepth 1 -type d \
    -exec echo {} ';' | sed -e 's/^.\///'); do
  bash $FEISTY_MEOW_SCRIPTS/whack_dupes_dir.sh $exemplar_dir/$i $i
done

# get back to where we started.
popd &>/dev/null

