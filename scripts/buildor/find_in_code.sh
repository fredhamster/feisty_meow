#!/bin/bash
seek="$1"; shift
if [ -z "$seek" ]; then
  echo This script needs a pattern to look for in the current directory.
  echo All code files here and in subdirectories will be searched for the
  echo pattern.
  exit 1
fi
#hmmm: might be nice to support multiple directories...
#      just need to pass them to find as an array maybe?
dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

find "$dir" -type f \( -iname "*" \
  ! -iname "*.class" \
  ! -iname "*.dll" \
  ! -iname "*.exe" \
  ! -iname "entries" \
  ! -iname "*.git" \
  ! -iname "*.gz" \
  ! -iname "*.jar" \
  ! -iname "*.lib" \
  ! -iname "*.obj" \
  ! -iname "*.svn" \
  ! -iname "*.svn-base" \
  ! -iname "*.tar" \
  ! -iname "*.tmp" \
  ! -iname "*.zip" \) \
  -exec echo "\"{}\"" ';' | xargs grep -li "$seek" 
