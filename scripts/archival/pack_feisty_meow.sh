#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

TEMPO_FILE="$(mktemp "$TMP/zz_feistypack.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
export XC='--exclude='

parent_dir="$(dirname "$FEISTY_MEOW_DIR")"
base_dir="$(basename "$FEISTY_MEOW_DIR")"

pushd $parent_dir

# zip up hoople2, but exclude the file names we never want to see.
tar -czf $TEMPO_FILE $base_dir $XC"*/*.tar.gz" $XC"*/*.zip" $XC"*/waste/*" $XC"*/logs/*" $XC"*/binaries/*" $XC"*.git*" 

# now move the newest version into its resting place.  this prepares the
# feisty_meow package for uploading.
mv -v $TEMPO_FILE $WEB_DIR/feistymeow.org/releases/feisty_meow_codebase_$(date_stringer).tar.gz

popd


