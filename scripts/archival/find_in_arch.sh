#!/bin/bash
##############
# Name   : find_in_arch
# Author : Chris Koeritz
# Rights : Copyright (C) 2012-$now by Feisty Meow Concerns, Ltd.
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############
#
# scours through the archive files (tar, zip, etc) in a directory looking for a pattern
# in the file.  any matches are reported.

dir="$1"; shift
pattern="$1"; shift

if [ -z "$dir" -o -z "$pattern" ]; then
  echo This utility requires a directory to scan for archives, and a pattern to
  echo look for within each archive.  Any matches are reported.
  exit 1
fi

TMPFILE="$(mktemp "$TMP/jarfinding.XXXXXX")"

#hmmm: below would be nicer if we had a util that told us all the types of archives
#      that we support.  then we could just skim across those types.

# locate all the archive files under the path.
find "$dir" -iname "*.jar" -o -iname "*.zip" -o -iname "*.tar" \
  -o -iname "*.iar" -o -iname "*.oar" \
  >"$TMPFILE"

while read line; do
  bash $FEISTY_MEOW_SCRIPTS/archival/listarch.sh "$line" 2>&1 | grep -i "$pattern" >/dev/null
  if [ $? -eq 0 ]; then echo ==== Found pattern in $line ====; fi
done <"$TMPFILE"

\rm -f "$TMPFILE"


