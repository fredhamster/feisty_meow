#!/usr/bin/env bash

if [ ! -d $RUNTIME_PATH/install ]; then mkdir $RUNTIME_PATH/install; fi

echo exe ending is $EXE_END

rm -f $RUNTIME_PATH/install/example_bundle$EXE_END

$EXECUTABLE_DIR/bundle_creator -o $RUNTIME_PATH/install/example_bundle$EXE_END -m ./example_manifest.txt

