#!/bin/bash

# copies the fallout 76 screenshots and images into our walruse metaverse hierarchy.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [[ "$(hostname)" =~ clemems* ]]; then
  character=chronical_pc
fi
if [[ "$(hostname)" =~ orpheus* ]]; then
  character=spoonburg_pc
fi
if [ -z "$character" ]; then
  echo Could not determine the proper character name to store the fallout 76
  echo images under.  Can you add a hostname to this script?
  echo "  => $0"
  exit 1
fi

src_dir="~/linx/wind_goods/My\ Games/Fallout\ 76/Photos/8f99c06443f04f6f8270604369bb78eb"
dest_dir="/z/walrus/media/pictures/metaverse/fallout_76/${character}"

if [ ! -d "$src_dir" ]; then
  echo "Could not find the source directory: $src_dir"
  exit 1
fi
if [ ! -d "$dest_dir" ]; then
  echo "Could not find the destination directory: $dest_dir"
  exit 1
fi

netcp "$src_dir"/*[0-9].png "$dest_dir"/


