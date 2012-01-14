#!/bin/bash

if [ ! -d $PRODUCTION_DIR/install ]; then mkdir $PRODUCTION_DIR/install; fi

echo exe ending is $EXE_END

rm -f $PRODUCTION_DIR/install/example_bundle$EXE_END

$EXECUTABLE_DIR/bundle_creator -o $PRODUCTION_DIR/install/example_bundle$EXE_END -m ./example_manifest.txt

