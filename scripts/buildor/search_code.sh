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

#hmmm: we need to encode this useful logic as a list of binary file endings.
find "$dir" -type f \( -iname "*" \
  ! -iname "*~" \
  ! -iname "*.bin" \
  ! -iname "*.bz2" \
  ! -iname "*.class" \
  ! -iname "*.dll" \
  ! -iname "*.deb" \
  ! -iname "*.dmg" \
  ! -iname "*.exe" \
  ! -iname "entries" \
  ! -iname "*.git" \
  ! -iname "*.gz" \
  ! -iname "*.iar" \
  ! -iname "*.jar" \
  ! -iname "*.lib" \
  ! -iname "*.oar" \
  ! -iname "*.obj" \
  ! -iname "*.png" \
  ! -iname "*.snarf" \
  ! -iname "*.so" \
  ! -iname "*.so.2" \
  ! -iname "*.svn" \
  ! -iname "*.svn-base" \
  ! -iname "*.tar" \
  ! -iname "*.tmp" \
  ! -iname "*.zip" \) \
  -exec echo "\"{}\"" ';' | xargs grep -li "$seek" | grep -v "^\.[^\/]\|\/\."


