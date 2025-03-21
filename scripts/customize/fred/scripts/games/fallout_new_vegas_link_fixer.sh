#!/usr/bin/env bash

# sets up links to make fallout new vegas work properly under wine.
# some mods will look for paths that are incorrectly case sensitive, and this
# script just sets up all those links that we believe are required.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ ! -d Data ]; then
  echo '
This script needs to run from the "Fallout New Vegas" install directory,
which is usually found under "steam/steamapps/common".
'
  exit 1
fi

function make_local_dir_link()
{
  dir="$1"; shift
  name="$1"; shift
  new_name="$1"; shift
  pushd "$dir" &>/dev/null
  if [ -L "$new_name" ]; then
    echo "Skipping creation of existing link $dir/$new_name"
    popd &>/dev/null
    return
  fi
  if [ ! -d "$name" ]; then
    echo "Skipping creation of link $dir/$new_name due to missing directory $dir/$name"
    popd &>/dev/null
    return
  fi
  ln -s "$name" "$new_name"
  exit_on_error "creating link for $dir/$new_name from $dir/$name"
  echo "Created link $dir/$new_name from $dir/$name"
  popd &>/dev/null
}

make_local_dir_link Data Sound sound
make_local_dir_link Data Textures textures
make_local_dir_link Data Meshes meshes
make_local_dir_link Data/Meshes Landscape landscape
make_local_dir_link Data/Meshes Weapons weapons
make_local_dir_link Data/Sound Voice voice
make_local_dir_link Data/Textures Landscape landscape
make_local_dir_link Data/Textures Landscape weapons
make_local_dir_link Data/Textures Landscape clutter

