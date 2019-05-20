#!/bin/bash

# copies the fallout 76 screenshots and images into our walruse metaverse hierarchy.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [[ "$(hostname)" =~ clemens* ]]; then
  character="chronical_pc"
  identity="8836c852c8a647ba8ca45808a73c3fbb"
elif [[ "$(hostname)" =~ orpheus* ]]; then
  character="spoonburg_pc"
  identity="8f99c06443f04f6f8270604369bb78eb"
else
  echo Could not determine the proper character name to store the fallout 76
  echo images under.  Can you add a hostname to this script?
  echo "  => $0"
  exit 1
fi

src_dir="$HOME/linx/wind_goods/My Games/Fallout 76/Photos/${identity}"
dest_dir="/z/walrus/media/pictures/metaverse/fallout_76/${character}"

#echo "src is: '$src_dir'"
#echo "dest is: '$dest_dir'"

if [ ! -d "$src_dir" ]; then
  echo "Could not find the source directory: $src_dir"
  exit 1
fi
if [ ! -d "$dest_dir" ]; then
  echo "Could not find the destination directory: $dest_dir"
  exit 1
fi

netcp "$src_dir"/*[0-9].png "$dest_dir"/


