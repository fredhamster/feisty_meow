#!/bin/bash

# updates my little 1 TB "soapbox" style usb drive with items that it should contain.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/archival/shared_updater_parts.sh"

# where we're backing up to.
TARGET_FOLDER="/media/fred/soapboxdrive"

sep

echo Target drive currently has...
ls "$TARGET_FOLDER"
if [ $? -ne 0 ]; then
  echo "The target location '$TARGET_FOLDER' is not mounted currently, so cannot be updated."
  exit 1
fi

# synch all our targets.
for currdir in $ARCHIVE_COLLECTION_LIST; do
  synch_directory_to_target "$currdir" "$TARGET_FOLDER/$(basename $currdir)"/
done

sep

# update source code if present.
echo getting latest fred repositories...
pushd "$TARGET_FOLDER"
for currdir in $SOURCE_HIERARCHY_LIST; do
  update_source_folders "$curr_dir"
done
sep

echo Updated all expected portions of the targets successfully.

