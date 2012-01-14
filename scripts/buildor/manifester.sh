#!/bin/bash
dir=$1
if [ -z "$dir" ]; then
  echo this command needs one directory as a parameter.  all the exes in that
  echo directory will be tagged with our security manifest.
  exit 23
fi

for i in $dir/*.exe; do
  mt -manifest $SHELLDIR/clam/cpp/security_manifest.txt -outputresource:$i\;\#1
done 

