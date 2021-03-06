#!/bin/bash

# cleans up the generated files that most people don't use.
# this includes some static libraries and all of the tests, as well as
# the install bundles.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
echo "Cleaning up feisty meow generated files..."

pushd $FEISTY_MEOW_GENERATED_STORE

\rm -rf logs clam_tmp temporaries/*

pushd runtime

\rm -rf install/*

pushd binaries

\rm -rf *.a *.library test_*

popd
popd
popd

echo "Finished with feisty meow generated file cleaning."
sep

