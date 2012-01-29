#!/bin/bash

# change to the home directory so we can find our files.
cd

TEMPO_FILE="$(mktemp "$TMP/zz_feistypack.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
export XC='--exclude='

# zip up hoople2, but exclude the file names we never want to see.
tar -czf $TEMPO_FILE feisty_meow $XC"*/*.tar.gz" $XC"*/*.zip" $XC"*/waste/*" $XC"*/logs/*" $XC"*/binaries/*" $XC"*/.git/*" 

date_string="$(date +"%Y_%b_%e_%H%M" | sed -e 's/ //g')"

# now move the newest version into its resting place.  this prepares the
# feisty_meow package for uploading.
mv -v $TEMPO_FILE $WEB_DIR/feistymeow.org/releases/feisty_meow_codebase_${date_string}.tar.gz

