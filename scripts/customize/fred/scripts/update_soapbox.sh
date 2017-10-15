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

#function synch_directory_to_target()
#{
#  local from="$1"; shift
#  local to="$1"; shift
#
#  sep
#
#  if [ ! -d "$from" ]; then
#    echo "skipping synch one missing source directory $from; this is not normal!"
#  fi
#  if [ ! -d "$to" ]; then
#    echo "skipping synch into non-existent directory $to"
#  fi
#
#  echo "synching from $from into $to"
#  netcp "$from"/* "$to"/
#  if [ $? -ne 0 ]; then
#    echo "The synchronization of $from into $to has failed."
#    exit 1
#  fi
#}

# do all our targets.
for currdir in $ARCHIVE_COLLECTIONS_LIST; do
  synch_directory_to_target "$currdir" "$TARGET_FOLDER/$(basename $currdir)"/
#  sep
#  echo "synching $currdir..."
#  netcp $currdir/* /media/fred/soapboxdrive/$currdir/
#  if [ $? -ne 0 ]; then
#    echo "The $currdir sync failed."
#    exit 1
#  fi
done

sep

echo getting latest fred repositories...
pushd "$TARGET_FOLDER"
update_source_folders extra_brain

sep

echo Updated all expected portions of the targets successfully.

