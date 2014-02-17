#!/bin/bash

# load in our helper functions, like date_stringer.
source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

# get into the genesis directory to package up.
pushd $GENII_INSTALL_DIR &>/dev/null

# clean out any class files or other generated junk.
echo Cleaning the code before pickling...
ant clean &>/dev/null

# do one update to get to the latest point.
echo Updating to latest code before pickling...
svn update . &>/dev/null

# get the svn revision number from the code now we're sure what prompt we can look for,
# modulo any check-ins since the above update was running.
svn_revision=$(svn update . | grep "At revision [0-9]*" | sed -e 's/[^0-9]*\([0-9]*\)[^0-9]*/\1/' )

filename="$HOME/GenesisII_source_rev${svn_revision}_on_$(date_stringer).tar.gz"

echo "Now pickling code into build archive: $(basename $filename)"

# hop up one level to be above the trunk folder.
cd ..

# now do the actual packing of the code, without some things we do not wish to share.
just_dir=$(basename $GENII_INSTALL_DIR)
#echo just dir is $just_dir
tar -czf $filename "$just_dir" --exclude="trunk/deployments/Genii*" --exclude="installer_base" --exclude=".svn" --exclude="products/*" --exclude="Media/*"

popd &>/dev/null

