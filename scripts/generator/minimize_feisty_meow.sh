#!/usr/bin/env bash

# cleans up the generated files that most people don't use.
# this includes some static libraries and all of the tests, as well as
# the install bundles.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
echo "Cleaning up feisty meow generated files..."

# first, we check to make sure our expected storage directory exists.
if [ ! -d $FEISTY_MEOW_GENERATED_STORE ]; then
  echo "
Problem seen during minimize--there is no generated store directory.
\$FEISTY_MEOW_GENERATED_STORE = '$FEISTY_MEOW_GENERATED_STORE'
"
  exit 1
fi

pushd $FEISTY_MEOW_GENERATED_STORE

# setting this flag to anything skips the sleep in the whackem script.
export WHACKEM_NO_SLEEPING=stayawake

whackem -rf logs clam_tmp temporaries/[a-zA-Z0-9]*

if [ -d runtime ]; then
  pushd runtime

  whackem -rf install/[a-zA-Z0-9]*

  if [ -d binaries ]; then
    pushd binaries
    whackem -rf *.a *.library test_*
    popd
  fi

  popd
fi

popd

echo "Finished with feisty meow generated file cleaning."
sep

