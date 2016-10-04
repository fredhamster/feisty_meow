#!/bin/bash

# Creates the application bundle of core code and support files.

version=$1

INSTDIR=$RUNTIME_PATH/install
if [ ! -d "$INSTDIR" ]; then mkdir "$INSTDIR"; fi

# the new packages are named after the build version.
total_build=$INSTDIR/whole_build_$version.exe
source_pack=$INSTDIR/sources_$version.exe
# clean out older versions of the packages.
rm -f $INSTDIR/whole_build_*exe $INSTDIR/sources_*exe

echo
echo Creating source package in `basename $source_pack`
$RUNTIME_PATH/binaries/bundle_creator -o $source_pack -m ./whole_build_manifest.txt --keyword sources

echo
echo Creating full build package in `basename $total_build`
$RUNTIME_PATH/binaries/bundle_creator -o $total_build -m ./whole_build_manifest.txt

