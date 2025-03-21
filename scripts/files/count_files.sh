#!/usr/bin/env bash

# counts the number of files in a set of directories.
# if no directories are provided, uses the current working directory.

# make sure they gave us some arguments, or go with our default of the current dir.
#hmmm: could use the count notation instead of sloppy empty check.
if [ -z "$1" ]; then
  # reset the arguments list to scan any directories found in cwd.
  SAVEIFS="$IFS"
  IFS=$(echo -en "\n\b")
  set -- $(find . -maxdepth 1 -mindepth 1 -type d) "${@:2}"
  IFS="$SAVEIFS"
fi

# run through all the parameters provided and find any directories under them
# (or probably barf if they're not dirs).
#hmmm: really?  "barf" is an implementation strategy now?
for countfilesname in "${@}" ; do
#  echo "arg is: '$countfilesname'"
  # print the count of files followed by directory name,
  # with leading zeros to allow sorting, which get
  # redigested as spaces before showing the list.
  printf "%06d -- %s\n" $(find "$countfilesname" -type f | wc -l) "$countfilesname"
done |
  # provide sorted output based on how many files exist
  # in each directory.
  sort -r |
  # eat the zeroes but keep the tabular look (i.e. replace each leading zero 
  # with a space).  had to do it as cases, since this seems like context-
  # sensitive matching, which sed will not do, i think).
  sed -e 's/^000000/     0/' -e 's/^00000/     /' -e 's/^0000/    /' \
      -e 's/^000/   /' -e 's/^00/  /' -e 's/^0/ /'

