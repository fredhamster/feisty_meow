#!/bin/bash

# a script that handles synchronization of important assets from the MAJOR_ARCHIVE_SOURCES
# and the SOURCECODE_HIERARCHY_LIST onto a backup drive of some sort.  it will only copy folders
# if there is a target folder of the appropriate name already on the backup medium.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# given a location in the filesystem, we will go to that location and attempt to
# update any revision control repositories stored there to the latest versions.
function update_source_folders()
{
  folder="$1"; shift
  if [ ! -d "$folder" ]; then
    echo "The folder '$folder' does not exist, so skipping repository update there."
    return;
  fi
  echo getting latest codes in $folder...
  pushd "$folder"
  if [ $? -ne 0 ]; then
    echo Changing to the folder $folder failed.
    exit 1
  fi
  bash "$FEISTY_MEOW_SCRIPTS/rev_control/rcheckin.sh"
  if [ $? -ne 0 ]; then
    echo Checking out the latest codes has failed somehow for $folder.
    exit 1
  fi
  popd
}

# this attempts to copy all the contents in a folder called "from" into a folder
# called "to".  it's a failure for the "from" folder to not exist, but the "to"
# is allowed to not exist (in which case we don't try to synch to it).
function synch_directory_to_target()
{
  local from="$1"; shift
  local to="$1"; shift

  sep

  if [ ! -d "$from" ]; then
    echo "skipping synch on missing source directory $from; this is not normal!"
    exit 1
  fi
  if [ ! -d "$to" ]; then
    echo "skipping synch into non-existent directory $to"
    return
  fi

  echo "synching from $from into $to"
  netcp "$from"/* "$to"/
  if [ $? -ne 0 ]; then
    echo "The synchronization of $from into $to has failed."
    exit 1
  fi
}

# the uber controller method that does the "hard" work of updating.
# any items from the MAJOR_ARCHIVE_SOURCES that are on the target will be
# updated.  any items found on the target matching the members of the
# SOURCECODE_HIERARCHY_LIST will be treated as code hierarchies and updated.
function update_archive_drive()
{
  local target_folder="$1"; shift  # where we're backing up to.
  local currdir  # loop variable.

  sep

  echo Target drive currently has...
  ls "$target_folder"
  if [ $? -ne 0 ]; then
    echo "The target location '$target_folder' is not mounted currently, so cannot be updated."
    exit 1
  fi

  # synch all our targets.
  for currdir in $MAJOR_ARCHIVE_SOURCES; do
    synch_directory_to_target "$currdir" "$target_folder/$(basename $currdir)"/
  done

  sep

  # update source code if present.
  echo getting latest fred repositories...
  pushd "$target_folder"
  for currdir in $SOURCECODE_HIERARCHY_LIST; do
    update_source_folders $currdir
  done
  
  sep

  echo successfully updated all expected portions of the target drive at:
  echo "  $target_folder"
}


