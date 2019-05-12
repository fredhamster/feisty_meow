#!/bin/bash

# compares the soapbox with the real archive to see if any older stuff might be
# left behind.  if it's got a less than in front, then it's only on the soapbox drive
# now rather than the pc's hard drive.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function compare_archives_with_target()
{
  local target="$1"; shift

  for currdir in $MAJOR_ARCHIVE_SOURCES; do
    sep
    echo "comparing '$currdir' with target '$target', where 'less thans' are on the target..."
    compare_dirs "$target/$(basename $currdir)" "$currdir"
  done
}

# decide which drive to compare.
targets="$1"
if [ -z "$targets" ]; then
  targets=($($(which ls) -1 /media/$USERNAME))
  if [ ${#targets[@]} -gt 1 ]; then
    echo "
Please provide a media drive name on the command line, because more than
one possibility exists.
"
  fi
fi

echo "comparing the media drive '${targets[0]}' against local archives."

compare_archives_with_target "/media/$USERNAME/${targets[0]}"

sep

