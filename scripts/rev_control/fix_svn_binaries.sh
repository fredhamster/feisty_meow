#!/bin/bash
# this script spiders across the first argument and locates any files that
# should be set up as binaries.  it removes the most likely wrong tags that
# these files have after conversion from cvs (using cvs2svn).
# we remove the executable bit because there are really very few files where
# we want this enabled in the source tree.
for i in `find $1 -type f -iname "*.a" \
  -o -iname "*.ansi" -o -iname "*.bat" -o -iname "*.bmp" \
  -o -iname "*.cur" -o -iname "*.dbc" -o -iname "*.dbf" \
  -o -iname "*.doc" -o -iname "*.dll" -o -iname "*.exe" \
  -o -iname "*.exp" -o -iname "*.gif" \
  -o -iname "*.gnucash" -o -iname "*.ico" -o -iname "*.jar" \
  -o -iname "*.jpg" -o -iname "*.lib" -o -iname "*.manifest" \
  -o -iname "*.mp3" \
  -o -iname "*.msi" -o -iname "*.ocx" -o -iname "*.pdf" \
  -o -iname "*.pm5" -o -iname "*.png" -o -iname "*.rtf" \
  -o -iname "*.wav" -o -iname "*.wri" -o -iname "*.vsd" \
  -o -iname "*.xcf" -o -iname "*.xls" -o -iname "*.zip" \
  `; do
  bash $SHELLDIR/svn_set_binary.sh "$i"
done

