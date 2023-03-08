#!/bin/bash
if [ -z "$1" -o -z "$2" ]; then
  echo You must supply a file name that has a movie database in it and a movie
  echo or show name to search for.  Example:
  echo 'movie_seeker "$HOME/moviedb.txt" "star trek"'
  exit 2
fi
SEEKING="$2"
if [ -z "$SEEKING" ]; then
  SEEKING='.\\*'
fi
#echo "pattern is $SEEKING"
cat "$1" | grep -v '","[^"]*","[^"]' |  sed -e 's/^\"\([a-zA-Z0-9][a-zA-Z0-9 ]*\)\",\"\([^\"][^\"]*\)\".*$/\2 [\1]/' | grep -i "$SEEKING" | sort | uniq 

