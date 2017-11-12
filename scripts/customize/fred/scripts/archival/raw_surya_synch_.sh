#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

echo "Updating walrus and musix from surya: raw mode without syncthing!"
echo

for currdir in basement imaginations musix walrus; do
  sep
  echo "synching $currdir folder..."
  rsync -avz surya:/z/$currdir/* /z/$currdir/
done

sep

