#!/usr/bin/env bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

outfile="$(mktemp "$TMP/region_ports.XXXXXX")"
grep "InternalPort *= *[0-9][0-9]*" "$HOME/opensim/bin/Regions"/* | sed -e "s/.*= *\([0-9]*\).*/\1/" | sort -g &>$outfile

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  sep
  echo all ports are:
  cat $outfile
  sep
  echo head
  head -1 $outfile
  sep
  echo tail
  tail -1 $outfile
  sep
fi

echo "opensim ports range from $(head -1 $outfile) to $(tail -1 $outfile)"
rm $outfile


