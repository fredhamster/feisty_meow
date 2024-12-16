#!/usr/bin/env bash

# strips out html tags and sends results to standard input.
function strip_html_code()
{
  for i in $*; do
    echo "----------------------------------------"
    echo "Text for web page: $i"
    sed -e 's/<[^>]*>//g' -e 's/  */ /g' -e 's/^ *//g' -e 's/^  *$//' <"$i" | grep -v '^$' 
    echo "----------------------------------------"
  done
}


