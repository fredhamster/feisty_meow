#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

synch_host="$1"; shift
if [ -z "$synch_host" ]; then
  echo This script requires a hostname where we expect to find archives.
  exit 1
fi

echo "Updating our local archives from $synch_host: this is very raw mode, without syncthing!" | splitter
echo

for currdir in basement imaginations musix walrus; do
  sep
  echo "synching $currdir folder from $synch_host..."
  rsync -avz "$synch_host":/z/$currdir/* /z/$currdir/
done

sep

