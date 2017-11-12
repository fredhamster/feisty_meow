#!/bin/bash

parm=$1

#echo original parm is: $parm
# turn the form that is just two characters of /X into X:/.
parm=$(echo $parm | sed -e 's/^\/\([a-zA-Z]\)$/\1:\//g')
#echo parm now is $parm
# turn the msys path form into an msdos style path for the drive letter.
parm=$(echo $parm | sed -e 's/^\/\([a-zA-Z]\)\//\1:\//g')
#echo parm now is $parm
# turn the form that is /cygdrive/X into X:/.
parm=$(echo $parm | sed -e 's/^\/cygdrive\/\([a-zA-Z]\)$/\1:\//g')
#echo parm now is $parm
# turn regular cygwin paths into msdos style paths for the drive letter.
parm=$(echo $parm | sed -e 's/^\/cygdrive\/\([a-zA-Z]\)\//\1:\//g')
#echo parm now is $parm
# rip off any slashes on the end, if they aren't too close to a colon.
parm=$(echo $parm | sed -e 's/\([^:]\)\/*$/\1/g')
#echo parm now is $parm
# turn linux forward slashes into dos backward slashes.
parm=$(echo $parm | sed -e 's/\//\\/g')
#echo "totally chewed parm is: $parm"

$WINDIR/explorer "$parm"

