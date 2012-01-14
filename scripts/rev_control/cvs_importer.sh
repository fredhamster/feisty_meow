#!/bin/bash
# cvs_importer: this tool does a recursive cvs add.  it takes one or more
# directory names and recursively adds them to the cvs hierarchy they are
# currently located under (i.e., their parent directory).

if [ "$#" = "0" ]; then
  echo "This program needs one or more directory names to recursively add."
  exit 23
fi

function add_cvs_dir {
  # loop across all the directories we were handed.  these had better be
  # arranged in hierarchically descending order or cvs will not like adding
  # them.
  for q in $*; do
    if [ "$(basename $q)" == "CVS"  ]; then
#echo "the parameter was cvs! -> $q"
      continue;
    fi
    cvs add "$q"
#echo "just added directory $q"
  done
}

function add_cvs_files {
  # scans through the list of directories we were given.  each directory is
  # assumed to have some files which need to be added.  we will add those
  # files to cvs on a per directory basis to avoid clogging up the command
  # lines and such.
  for q in $*; do
    if [ "$(basename $q)" == "CVS"  ]; then
#echo "skipping parameter as cvs! -> $q"
      continue;
    fi

    pushd $q &>/dev/null
    if [ $? -ne 0 ]; then
      echo "Skipping badly erroneous directory $i.  Logic error?"
      continue;
    fi

    # add all the files in this directory, but don't do subdirectories.
    file_list=$(find . -maxdepth 1 -type f)
    # make sure there are actually some files there.
    if [ ! -z "$file_list" ]; then
      find . -maxdepth 1 -type f -exec cvs add "{}" '+'
#echo "just added those files to $q directory"
    fi

    # go back to where we were before jumping into the directory.
    popd &>/dev/null
  done
}

# main activity of script occurs starting here...

# loop across the directory names we were given on the command line.
for i in $*; do
  # change into the directory just above the directory to add.
  parent_dir=$(dirname $i)
  adding_dir=$(basename $i)
  pushd $parent_dir &>/dev/null
  if [ $? -ne 0 ]; then
    echo "Skipping erroneous parent directory $parent_dir."
    continue;
  fi
#echo dir is now: $(pwd)

  # find all the directories starting at the real directory to add, and add
  # cvs repositories for each of them.
  add_cvs_dir $(find $adding_dir -type d)

  # now add all the files, since all of the directories should be listed
  # in the cvs archive.
  add_cvs_files $(find $adding_dir -type d) 

  # go back to the directory where we were previously.
  popd &>/dev/null

done

