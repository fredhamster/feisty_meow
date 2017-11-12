#!/bin/bash

# a script that handles synchronization of important assets from the ARCHIVE_COLLECTION_LIST
# and the SOURCE_HIERARCHY_LIST onto a backup drive of some sort.  it will only copy folders
# if there is a target folder of the appropriate name already on the backup medium.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/archival/shared_updater_parts.sh"

function update_archive_drive()
{
  local target_folder="$1"; shift
    # where we're backing up to.

  sep

  echo Target drive currently has...
  ls "$target_folder"
  if [ $? -ne 0 ]; then
    echo "The target location '$target_folder' is not mounted currently, so cannot be updated."
    exit 1
  fi

  # synch all our targets.
  for currdir in $ARCHIVE_COLLECTION_LIST; do
    synch_directory_to_target "$currdir" "$target_folder/$(basename $currdir)"/
  done

  sep

  # update source code if present.
  echo getting latest fred repositories...
  pushd "$target_folder"
  update_source_folders $SOURCE_HIERARCHY_LIST
#hmmm:clean
#extra_brain interbrane
#need source list
  
  sep

  echo Updated all expected portions of the targets successfully.
}


