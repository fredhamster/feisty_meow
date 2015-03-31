#!/bin/bash
seek="$1"; shift
if [ -z "$seek" ]; then
  echo This script needs a pattern to look for in the current directory.
  echo All non-binary files here and in subdirectories will be searched for the
  echo pattern.
  echo A directory can be passed as an optional second parameter, which will
  echo cause the search to occur in that directory instead.
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
  ! -iname "*.doc" \
  ! -iname "*.docx" \
  ! -iname "*.exe" \
  ! -iname "entries" \
  ! -iname "*.git" \
  ! -iname "*.gif" \
  ! -iname "*.gz" \
  ! -iname "*.iar" \
  ! -iname "*.jar" \
  ! -iname "*.jpg" \
  ! -iname "*.lib" \
  ! -iname "*.ncb" \
  ! -iname "*.oar" \
  ! -iname "*.obj" \
  ! -iname "*.pch" \
  ! -iname "*.png" \
  ! -iname "*.snarf" \
  ! -iname "*.so" \
  ! -iname "*.so.2" \
  ! -iname "*.svn" \
  ! -iname "*.svn-base" \
  ! -iname "*.tar" \
  ! -iname "*.tmp" \
  ! -iname "*.xls" \
  ! -iname "*.xlsx" \
  ! -iname "*.zip" \) \
  -exec echo "\"{}\"" ';' | xargs grep -li --binary-files=without-match -- "$seek" | grep -v "^\.[^\/]\|\/\."

# first grep looks in every valid file for the pattern requested.
# second grep strains out dot files.
#hmmm: why are we doing that second step?



