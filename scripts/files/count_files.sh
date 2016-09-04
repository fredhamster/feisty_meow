#!/bin/bash


# make sure they gave us some arguments.
if [ -z "$1" ]; then
#hmmm: could use the count notation instead of sloppy empty check.
  # reset first arg to be '*' to do any directories here.
#echo changing args to use all subdirs in current dir.
  set -- $(find . -maxdepth 1 -mindepth 1 -type d) "${@:2}"
#echo "arg 1 is now '$1'"
fi

# run through all the parameters provided and find any
# directories under them (or probably barf if they're not
# dirs).
for i in "${@}" ; do
  # print the count of files followed by directory name,
  # with leading zeros to allow sorting, which get
  # redigested as spaces before showing the list.
  printf "%06d -- %s\n" $(find "$i" -type f | wc -l) "$i"
done |
# provide sorted output based on how many files exist
# in each directory.
  sort -r |
# eat the zeroes but keep the tabular look.  this simple
# sed code will eat zeroes in names also.  oops.
  sed -e 's/0/ /g'

