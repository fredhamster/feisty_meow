#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

TEMPO_FILE="$(mktemp "$TMP/zz_feistypack.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
export XC='--exclude='

parent_dir="$(dirname "$FEISTY_MEOW_APEX")"
base_dir="$(basename "$FEISTY_MEOW_APEX")"

pushd $parent_dir

# zip up feisty meow, but exclude the file names we never want to see.
tar -h -czf $TEMPO_FILE $base_dir $XC"*/*.tar.gz" $XC"*/*.zip" $XC"*/waste/*" $XC"*/logs/*" $XC"*/binaries/*" $XC"*.git*" $XC"*/code_guide/*" $XC"*/kona/bin/*"

# now move the newest version into its resting place.  this prepares the
# feisty_meow package for uploading.
mv -v $TEMPO_FILE $WEBBED_SITES/feistymeow.org/releases/feisty_meow_codebase_$(date_stringer).tar.gz

popd


