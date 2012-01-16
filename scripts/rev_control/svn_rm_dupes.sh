#!/bin/bash

# whacks the files in the current directory which are duplicates of the
# files in the directory passed as a parameter.
# if there is a second parameter, then it is used as the "current directory".

exemplar_dir="$1"; shift
whack_dir="$1"; shift

#hmmm: much code here is shared with whack_dupes.  get into a library.

# make sure they gave us a good directory to start with.
if [ -z "$exemplar_dir" ]; then
  echo "whack_dupes"
  echo "-----------"
  echo ""
  echo "This program needs at least one directory parameter.  The files in the"
  echo "current directory will be removed if a file in the specified directory"
  echo "already exists.  So... the current directory is the less important one"
  echo "and is presumed to have duplicates AND the directory given as parameter"
  echo "is considered important and has the best versions of the files."
  echo "If there is an optional second parameter, then that is used as the"
  echo "\"current\" directory where we start from; it will be the less important"
  echo "directory and will have its entries cleaned if they're duplicates."
  exit 42;
fi

# check to make sure they gave us a good directory.
if [ ! -z "$whack_dir" -a ! -d "$whack_dir" ]; then
  echo "the directory $whack_dir does not exist."
  exit 3
fi

# test the tasty remote location with the better contents.
pushd "$exemplar_dir" &>/dev/null
if [ $? -ne 0 ]; then
  # an error getting to this directory means its no good for us.
  echo "the directory $exemplar_dir is inaccessible."
  exit 2
fi
the_good_place="$(pwd)"
popd &>/dev/null

if [ ! -z "$whack_dir" ]; then
  # use the directory as our "current" location.
  pushd "$whack_dir" &>/dev/null
fi

# now that we're in the directory to clean, make sure we're good there.
if [ ! -d ".svn" ]; then
#  echo "Could not find a subversion directory; operation would be pointless."
  exit 0
fi

current_dir="$(pwd)"

#echo "currdir=$current_dir gooddir=$the_good_place"

if [ "$current_dir" == "$the_good_place" ]; then
  # this is not good; they're the same location.
  echo "the request would whack all the files in the current directory; ignoring."
  exit 4
fi

# do the real work now...
for i in *; do
  if [ -f "$exemplar_dir/$i" ]; then
    echo "whacking $i"
    svn rm "$i"
  fi
done

if [ ! -z "$whack_dir" ]; then
  popd &>/dev/null
fi

