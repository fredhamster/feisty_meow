#!/bin/bash

# copies the fallout 76 screenshots and images into our walrus metaverse hierarchy.

# pull in the feisty scripts.
source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# figure out where we are.  the script must have at least one side be local,
# so we are planning for that to be the source side.
source_host="$(hostname)"

# constant-like variables...
DESTINATION_HOST=greendragon

# decide where we're copying based on the source.
if [[ "$source_host" =~ clemens* ]]; then
  character="chronical_pc"
  identity="8836c852c8a647ba8ca45808a73c3fbb"
elif [[ "$source_host" =~ orpheus* ]]; then
  character="spoonburg_pc"
  identity="8f99c06443f04f6f8270604369bb78eb"
else
  echo "
Could not determine the proper character name for the fallout 76 images based
on your source host of '$source_host' .  Can you add the details of your hostname
to this script?
  => script is at '$0'
"
  exit 1
fi

src_dir="$HOME/data/wind_goods/My Games/Fallout 76/Photos/${identity}"
dest_dir="fred@${DESTINATION_HOST}:/z/walrus/media/pictures/metaverse/fallout_76/${character}"

echo "src is: '$src_dir'"
echo "dest is: '$dest_dir'"

if [ ! -d "$src_dir" ]; then
  echo "Could not find the source directory: $src_dir"
  exit 1
fi
#if [ ! -d "$dest_dir" ]; then
#  echo "Could not find the destination directory: $dest_dir"
#  exit 1
#fi

netcp "$src_dir"/*[0-9].png "$dest_dir"/


