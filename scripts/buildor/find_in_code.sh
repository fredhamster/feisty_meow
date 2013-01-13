#!/bin/bash
seek="$1"; shift
if [ -z "$seek" ]; then
  echo This script needs a pattern to look for in the current directory.
  echo All code files here and in subdirectories will be searched for the
  echo pattern.
  exit 1
fi

find . -type f \( -iname "*" \
  ! -iname "*.svn" \
  ! -iname "*.git" \
  ! -iname "*.exe" \
  ! -iname "*.obj" \
  ! -iname "*.class" \
  ! -iname "*.dll" \
  ! -iname "*.lib" \
  ! -iname "*.jar" \
  ! -iname "*.zip" \
  ! -iname "*.tar" \
  ! -iname "*.gz" \) \
  -exec echo "\"{}\"" ';' | xargs grep -li "$seek" 
