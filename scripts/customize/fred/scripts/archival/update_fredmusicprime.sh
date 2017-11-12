#!/bin/bash

# updates my little 1 TB "soapbox" style usb drive with items that it should contain.

source "$FEISTY_MEOW_SCRIPTS/archival/general_updater.sh"

update_archive_drive "/media/fred/fredmusicprime"

exit $?

#####old#####old#####
#gone below.

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
update_source_folders $SOURCE_HIERARCHY_LIST

sep

echo Updated all expected portions of the targets successfully.

