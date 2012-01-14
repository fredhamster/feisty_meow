#!/bin/bash

parm=$1

#echo parm original is: $parm
# turn the form that is just two characters of /X into X:/.
parm2=$(echo $parm | sed -e 's/^\/\([a-zA-Z]\)$/\1:\//g')
#echo parm2 is $parm2
# turn the msys path form into an msdos style path for the drive letter.
parm3=$(echo $parm2 | sed -e 's/^\/\([a-zA-Z]\)\//\1:\//g')
#echo parm3 is $parm3
# rip off any slashes on the end, if they aren't too close to a colon.
parm4=$(echo $parm3 | sed -e 's/\([^:]\)\/*$/\1/g')
#echo parm4 is $parm4
# turn linux forward slashes into dos backward slashes.
parm5=$(echo $parm4 | sed -e 's/\//\\/g')
#echo "chewed parm5 is: $parm5"

$WINDIR/explorer "$parm5"

