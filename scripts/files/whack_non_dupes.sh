#!/usr/bin/env bash

# whacks the files in the current directory which are NOT duplicates of the
# files in the directory passed as a parameter.
# if there is a second parameter, then it is used as the "current directory"
# and it will be the target of any deletions.

exemplar_dir="$1"; shift
whack_dir="$1"; shift

# make sure they gave us a good directory to start with.
if [ -z "$exemplar_dir" ]; then
  echo "$(basename $0 .sh): This program needs at least one directory parameter."
  echo "The files in the current directory will be removed if a file in the specified directory"
  echo "does not exist.  So... the current directory is the less important one and is presumed"
  echo "to have rogue files AND the directory given as parameter is considered important and has"
  echo "the best canonical versions of the files."
  echo "If there is an optional second parameter, then that is used as the"
  echo "\"current\" directory where we start from; it will be the less important"
  echo "directory and will have its entries cleaned if they're non-duplicates."
  exit 1
fi

# check to make sure they gave us a good directory.
if [ ! -z "$whack_dir" -a ! -d "$whack_dir" ]; then
  echo "the directory $whack_dir does not exist."
  exit 1
fi

# test the tasty remote location with the better contents.
pushd "$exemplar_dir" &>/dev/null
if [ $? -ne 0 ]; then
  # an error getting to this directory means its no good for us.
  echo "the directory $exemplar_dir is inaccessible."
  exit 1
fi
the_good_place="$(pwd)"
popd &>/dev/null

if [ ! -z "$whack_dir" ]; then
  # use the directory as our "current" location.
  pushd "$whack_dir" &>/dev/null
fi

current_dir="$(pwd)"

#echo "currdir=$current_dir gooddir=$the_good_place"

if [ "$current_dir" == "$the_good_place" ]; then
  # this is not good; they're the same location.
  echo "the request would whack all the files in the current directory; ignoring."
  exit 1
fi

# do the real work now...
for i in *; do
  if [ ! -f "$exemplar_dir/$i" ]; then
    echo "whacking $i"
    rm -f "$i"
  fi
done

if [ ! -z "$whack_dir" ]; then
  popd &>/dev/null
fi

