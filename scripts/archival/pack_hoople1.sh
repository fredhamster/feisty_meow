#!/bin/bash

# change to the home directory so we can find our files.
cd

TEMPO_FILE="$(mktemp "$TMP/zz_temp_hoople1.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
export XC='--exclude='

# zip up hoople1, but exclude the file names we never want to see.
tar -czf $TEMPO_FILE hoople1 $XC"*CVS/*" $XC"*.svn/*" $XC"*/*.tar.gz" $XC"*/*.zip" $XC"*/3rdparty/*" $XC"hoople1/dll/*" $XC"hoople1/exe/*" $XC"hoople1/lib/*" $XC"hoople1/tests/*" $XC"hoople1/install/*" $XC"hoople1/install/*" $XC"hoople1/include/*" $XC"hoople1/logs/*" $XC"hoople1/objects/*" $XC"hoople1/docs/html/*" $XC"hoople1/binaries/*" $XC"hoople1/waste/*"

# now move the newest version into its resting place.  this prepares the
# hoople package for uploading.
mv -v $TEMPO_FILE /var/www/hoople.org/hoople1_library.tar.gz

