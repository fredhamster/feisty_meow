#!/bin/bash

cd /var/www/hoople.org
  # change to the hoople directory.

export TEMPO_FILE="$(mktemp "$TMP/zz_temp_msys_bins.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

export XC='--exclude='

tar -czf $TEMPO_FILE hoople/msys $XC"*.svn/*" $XC"*CVS/*" $XC"*/*.tar.gz" $XC"*/*.zip" 
  # exclude the file names we never want to see.

# move the file into its well-known location.
mv -v $TEMPO_FILE msys_bins.tar.gz

