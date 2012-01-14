#!/bin/bash

# shows the directory passed as the first parameter.
# if there's a second parameter, it's used for a filtering pattern.

function send_header()
{
  echo "Content-type: text/html"
  echo ""
  echo ""
}

dir="$1"; shift
pattern="*";
temp_pat="$1"; shift
if [ ! -z "$temp_pat" ]; then
  # we don't allow any clever scooting up the directory hierarchy...
  if [ -z "$(echo "$temp_pat" | grep "\.\.")" ]; then
    pattern="$temp_pat"
  fi
fi

# check that they've at least provided a directory.
if [ -z "$dir" ]; then
  send_header
  echo "$(basename $0): This needs a directory name before it can show the folder."
  exit
fi

# make sure they aren't trying to go above the web root.
if [ ! -z "$(echo "$dir" | grep "\.\.")" ]; then
  echo "$(basename $0): Will not go above the root web directory."
  exit
fi


send_header

echo "[$dir]"
echo "<br>"

fulldir="/var/www/$dir"

for i in "$fulldir"/$pattern; do
  dirlisting="$(ls -dsh $i | sed -e 's/ \/.*$//')"
  echo "<a href=\"$(basename $i)\">$(basename $i) ($dirlisting)</a> <br>"
done

popd &>/dev/null


