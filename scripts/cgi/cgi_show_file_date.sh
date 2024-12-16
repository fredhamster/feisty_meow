#!/usr/bin/env bash

# shows the date on a particular file.

filename=$(echo $QUERY_STRING | sed -e "s/\+/ /g" )

echo "Content-type: text/plain"
echo ""
echo ""
#echo "query string is $QUERY_STRING"
#echo "filename=$filename"
date -r "$filename" +"%A %B %d %Y %T %p" | tr -d '/\n/'

