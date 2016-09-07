#!/bin/bash

# a helpful script that scrapes any active game saves from wine's storage
# area into a spooling saves folder for archiving.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

WINE_SOURCE_DIR="$HOME/wine_goods/My Games"
SPOOLING_OUTPUT_DIR="$HOME/spooling_saves"

# copies the files for a particular game out to a spooling folder.
function copyem()
{
  game_name="$1"; shift
  source_dir="$1"; shift
  out_dir="$1"; shift

  if [ -d "$source_dir" ]; then
    echo $game_name
    cp -v -n "$source_dir"/* "$out_dir"/
    sep 28
  fi
}

# make the output folders if they don't exist.
for i in skyrim fallout_new_vegas fallout_3 oblivion fallout4 ; do
  if [ ! -d "$SPOOLING_OUTPUT_DIR/$i" ]; then
    mkdir -p "$SPOOLING_OUTPUT_DIR/$i"
  fi
done

# now run through and copy our save files from the potentially weird locations
# they reside in.

#hmmm: at least make a function out of those repetitive steps.  sheesh.

sep 28

copyem "skyrim" "$WINE_SOURCE_DIR/Skyrim/Saves" "$SPOOLING_OUTPUT_DIR/skyrim"

copyem "fallout new vegas" "$WINE_SOURCE_DIR/FalloutNV/Saves" "$SPOOLING_OUTPUT_DIR/fallout_new_vegas"

copyem "fallout 3" "$WINE_SOURCE_DIR/Fallout3/Saves/Player1" "$SPOOLING_OUTPUT_DIR/fallout_3"

copyem "oblivion" "$WINE_SOURCE_DIR/Oblivion/Saves" "$SPOOLING_OUTPUT_DIR/oblivion/"

copyem "fallout 4" "$WINE_SOURCE_DIR/Fallout4/Saves" "$SPOOLING_OUTPUT_DIR/fallout4"


