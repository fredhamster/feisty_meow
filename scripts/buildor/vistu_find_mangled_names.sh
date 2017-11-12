#!/bin/bash
# this script takes one microsoft compiler output file and tries to find the
# mangled C++ function names contained in it.
file=$1
dumpbin //all "$file" | grep SECREL | sed -e 's/^.*0000C *[0-9a-fA-F][0-9a-fA-F] *\([^ ]*\).*$/\1/' 

