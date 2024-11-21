#!/bin/bash

# copies the fallout 76 screenshots and images into our walrus metaverse hierarchy.

# pull in the feisty scripts.
source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# constant-like variables...
DESTINATION_HOST=greendragon

# copies all photos found for the steam identity, which we store under the
# character's name in our remote storage folder.
function copy_f76_photos_from_src_to_dest()
{
  local src_top="$1"; shift
  local identity="$1"; shift
  local character="$1"; shift
  local dest_top="$1"; shift

  if [ -z "$src_top" -o -z "$identity" -o -z "$character" -o -z "$dest_top" ]; then
    echo "Not enough parameters in the copy_f76_photos_from_src_to_dest function."
    exit 1
  fi
 
  local src_dir="$src_top/$identity"
  local dest_dir="fred@${DESTINATION_HOST}:$dest_top/${character}"

echo "src is: '$src_dir'"
echo "dest is: '$dest_dir'"

  if [ ! -d "$src_dir" ]; then
    echo "(No photo directory found for $character)"
    return 1
  fi

  echo "Copying photos for $character..."
  netcp "$src_dir"/*[0-9].png "$dest_dir"/
  if [ $? -ne 0 ]; then
    echo A problem occurred during the copy.
    return 1
  fi
}

# top directories for our home rig.
src_top="$HOME/data/photos_fallout76"
dest_top="/z/walrus/media/pictures/metaverse/fallout_76"

if [ ! -d "$src_top" ]; then
  echo "
The source directory:
$src_dir
cannot be located.  Please ensure that exists.
It is tricky, and needs to be based on the game's
secret number in steam.  For example, I set this up recently
on cuboid with:
ln -s \"/home/fred/.steam/steam/steamapps/compatdata/1151340/pfx/drive_c/users/steamuser/Documents/My Games/Fallout 76/Photos\" ~/data/photos_fallout76
Gnarly, but simpler than older schemes.
"
  exit 1
fi

# now run through our permutations for users...

copy_f76_photos_from_src_to_dest "$src_top" "8836c852c8a647ba8ca45808a73c3fbb" "chronical_pc" "$dest_top"

copy_f76_photos_from_src_to_dest "$src_top" "8f99c06443f04f6f8270604369bb78eb" "spoonburg_pc" "$dest_top"

