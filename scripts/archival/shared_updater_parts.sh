#!/bin/bash

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
  bash "$FEISTY_MEOW_SCRIPTS/rev_control/rev_checkin.sh"
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

