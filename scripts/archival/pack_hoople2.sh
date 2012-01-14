#!/bin/bash

# change to the home directory so we can find our files.
cd

TEMPO_FILE="$(mktemp "$TMP/zz_temp_hoople2.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
export XC='--exclude='

# zip up hoople2, but exclude the file names we never want to see.
tar -czf $TEMPO_FILE hoople2 $XC"*CVS/*" $XC"*.svn/*" $XC"*/*.tar.gz" $XC"*/*.zip" $XC"*/3rdparty/*" $XC"hoople2/install/*" $XC"hoople2/logs/*" $XC"hoople2/docs/html/*" $XC"hoople2/binaries/*" $XC"hoople2/waste/*"

# now move the newest version into its resting place.  this prepares the
# hoople2 package for uploading.
mv -v $TEMPO_FILE /var/www/hoople.org/hoople2_library.tar.gz

