#!/usr/bin/env bash

# an attempt to have a "fast" counter of the number of files in a folder, or folders, which will not stray into any snapshot directories present in the filesystem.

for dir in "${@}"; do
  echo -n "files in '$dir': "
  # seek out all files under the directory, except ones under the snapshot folder (pattern will match snapshots for either netapp (.snapshot) or dell (.snapshots).
  find "$dir" -iname ".snapshot" -prune -or -iname "*" -type f | grep -v "\.snapshot" | wc -l

#this implementation is the former one, and does not avoid traversing the .snapshot folder on netapp, thus making it super inefficient and complainy.
#the new implementation above is necessitated by needing to avoid multiplying our task by maybe 30 times, due to the snapshots looking mostly just like the original and having just as many files, in general.
#  /bin/ls -1fR "$dir" | grep -v "^$" | grep -v "^\.$" | grep -v "^\.\.$" | grep -v ".*:$" | grep -v "\.snapshot" | wc -l
#    # patterns that remove files from being counted, above:
#    #
#    # ^$           -- all blank lines
#    # ^\.$         -- all lines with just a dot (current directory)
#    # ^\.\.$       -- all lines with just two dots (parent directory)
#    # .*:$         -- all lines that end with a colon (directory heading from recursive ls)
#    # \.snapshot   -- all lines mentioning the snapshot directory.

done


