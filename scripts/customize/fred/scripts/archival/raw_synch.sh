#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

synch_host="$1"; shift
if [ -z "$synch_host" ]; then
  echo This script requires a hostname where we expect to find archives.
  exit 1
fi

echo "Updating our local archives from $synch_host: this is very raw mode, without syncthing!" | splitter
echo

for currdir in basement imaginations musix toaster walrus; do
  sep
  echo "synching $currdir folder from $synch_host..."
  if [ -d "/z/$currdir" ]; then
    rsync -avz "$synch_host":/z/$currdir/* /z/$currdir/
    continue_on_error synching with $currdir from remote host.
  fi
done

sep

