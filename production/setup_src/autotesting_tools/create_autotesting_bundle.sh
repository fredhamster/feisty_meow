#!/bin/bash

# Creates a bundle of tools for testing to improve our capabilities in
# scripting.

INSTDIR=$REPOSITORY_DIR/install/Tests
if [ ! -d "$INSTDIR" ]; then mkdir -p "$INSTDIR"; fi
TOOL_PACK=$INSTDIR/autotesting_tools.exe
rm -f "$TOOL_PACK"

$REPOSITORY_DIR/binaries/bundle_creator -o "$TOOL_PACK" -m ./autotesting_manifest.txt
if [ "$?" != "0" ]; then
  echo Failure during creation of autotest package.
  exit 23
fi

