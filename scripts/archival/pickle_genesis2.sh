#!/bin/bash

# load in our helper functions, like date_stringer.
source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

# get into the genesis directory to package up.
pushd $HOME/xsede/code/genesis2/trunk &>/dev/null

# clean out any class files or other generated junk.
ant clean &>/dev/null

# do one update to get to the latest point.
svn update . &>/dev/null

# get the svn revision number from the code now we're sure what prompt we can look for,
# modulo any check-ins since the above update was running.
svn_revision=$(svn update . | grep "At revision [0-9]*" | sed -e 's/[^0-9]*\([0-9]*\)[^0-9]*/\1/' )

filename="$HOME/GenesisII_source_rev${svn_revision}_on_$(date_stringer).tar.gz"

echo "Creating build archive: $(basename $filename)"

# hop up one level to be above the trunk folder.
cd ..

# now do the actual packing of the code, without some things we do not wish to share.
tar -czf $filename trunk/ --exclude="trunk/deployments/Genii*" --exclude="installer_base" --exclude=".svn" --exclude="products/*" --exclude="Media/*"

popd &>/dev/null


