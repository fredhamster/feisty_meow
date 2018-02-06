

# find all the directories at this height.
find . -mindepth 1 -maxdepth 1 -type d -exec echo {} ';' >$TMP/filestolink.txt 
# make links with all lower case and all upper case versions of the names.
while read line; do
  ln -s "$line" "$(echo $line | tr '[:upper:]' '[:lower:]')"
  ln -s "$line" "$(echo $line | tr '[:lower:]' '[:upper:]')"
done < $TMP/filestolink.txt 
# remove dead links.
\rm $(find . -type l ! -exec test -e {} \; -print)


