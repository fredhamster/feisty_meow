#!/bin/bash

# compares the soapbox with the real archive to see if any older stuff might be
# left behind.  if it's got a less than in front, then it's only on the soapbox drive
# now rather than the pc's hard drive.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function compare_archives_with_target()
{
  local target="$1"; shift

  for currdir in $ARCHIVE_COLLECTION_LIST; do
    sep
    echo "comparing '$currdir' with target '$target', where 'less thans' are on the target..."
    compare_dirs "$target/$(basename $currdir)" "$currdir"
  done
}

compare_archives_with_target /media/fred/soapboxdrive

sep

