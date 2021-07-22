

for dir in "${@}"; do
  echo -n "files in '$dir':"
  /bin/ls -1fR "$dir" | grep -v "^$" | grep -v "^\.$" | grep -v "^\.\.$" | grep -v ".*:$" | grep -v "\.snapshot" | wc -l

    # patterns that remove files from being counted, above:
    #
    # ^$           -- all blank lines
    # ^\.$         -- all lines with just a dot (current directory)
    # ^\.\.$       -- all lines with just two dots (parent directory)
    # .*:$         -- all lines that end with a colon (directory heading from recursive ls)
    # \.snapshot   -- all lines mentioning the snapshot directory.
done


