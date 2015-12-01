#!/bin/bash

if [ ! -d $RUNTIME_DIR/install ]; then mkdir $RUNTIME_DIR/install; fi

echo exe ending is $EXE_END

rm -f $RUNTIME_DIR/install/example_bundle$EXE_END

$EXECUTABLE_DIR/bundle_creator -o $RUNTIME_DIR/install/example_bundle$EXE_END -m ./example_manifest.txt

