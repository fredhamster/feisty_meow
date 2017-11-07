#!/bin/bash

# a helpful script that scrapes any active game saves from wine's storage
# area into a spooling saves folder for archiving.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

#we are missing the netcp thing.  why????

WINE_SOURCE_DIR="$HOME/wine_goods/My Games"
SPOOLING_OUTPUT_DIR="$HOME/data/spooling_saves"

if [ ! -d "$WINE_SOURCE_DIR" ]; then
  WINE_SOURCE_DIR="/cygdrive/c/users/fred/My Documents/My Games"
fi
if [ ! -d "$WINE_SOURCE_DIR" ]; then
  echo "Failing to find the game save directories."
  exit 1
fi

if [ ! -d "$SPOOLING_OUTPUT_DIR" ]; then
  mkdir -p "$SPOOLING_OUTPUT_DIR"
  test_or_fail Creating spooling output directory.
fi

# copies the files for a particular game out to a spooling folder.
function copyem()
{
  game_name="$1"; shift
  source_dir="$1"; shift
  out_dir="$1"; shift

  if [ -d "$source_dir" ]; then
    echo $game_name
#old    cp -v -n "$source_dir"/* "$out_dir"/
    netcp "$source_dir"/* "$out_dir"/
    sep 28
  fi
}

# make the output folders if they don't exist.
for i in skyrim fallout_new_vegas fallout_3/Saves oblivion fallout_4/Saves ; do
  if [ ! -d "$SPOOLING_OUTPUT_DIR/$i" ]; then
    mkdir -p "$SPOOLING_OUTPUT_DIR/$i"
  fi
done

# now run through and copy our save files from the potentially weird locations
# they reside in.

sep 28

copyem "skyrim" "$WINE_SOURCE_DIR/Skyrim/Saves" "$SPOOLING_OUTPUT_DIR/skyrim"

copyem "fallout new vegas" "$WINE_SOURCE_DIR/FalloutNV/Saves" "$SPOOLING_OUTPUT_DIR/fallout_new_vegas"

copyem "fallout 3" "$WINE_SOURCE_DIR/Fallout3/Saves" "$SPOOLING_OUTPUT_DIR/fallout_3/Saves"

copyem "oblivion" "$WINE_SOURCE_DIR/Oblivion/Saves" "$SPOOLING_OUTPUT_DIR/oblivion/"

copyem "fallout 4" "$WINE_SOURCE_DIR/Fallout4/Saves" "$SPOOLING_OUTPUT_DIR/fallout_4/Saves"

