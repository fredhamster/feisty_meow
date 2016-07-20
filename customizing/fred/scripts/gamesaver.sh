#!/bin/bash

# a helpful script that scrapes any active game saves from wine's storage
# area into a spooling saves folder for archiving.

WINE_SOURCE_DIR="$HOME/wine_goods/My Games"
SPOOLING_OUTPUT_DIR="$HOME/spooling_saves"

function line()
{
  echo "======="
}

# make the output folders if they don't exist.
for i in oblivion fallout_new_vegas fallout_3 skyrim ; do
  if [ ! -d "$SPOOLING_OUTPUT_DIR/$i" ]; then
    mkdir -p "$SPOOLING_OUTPUT_DIR/$i"
  fi
done

# now run through and copy our save files from the potentially weird
# locations they reside in.
line
echo skyrim
cp -v -n "$WINE_SOURCE_DIR/Skyrim/Saves"/* "$SPOOLING_OUTPUT_DIR/skyrim/"
line
echo fallout new vegas
cp -v -n "$WINE_SOURCE_DIR/FalloutNV/Saves"/* "$SPOOLING_OUTPUT_DIR/fallout_new_vegas/"
line
echo fallout 3
cp -v -n "$WINE_SOURCE_DIR/Fallout3/Saves/Player1"/* "$SPOOLING_OUTPUT_DIR/fallout_3/"
line
echo oblivion
cp -v -n "$WINE_SOURCE_DIR/Oblivion/Saves"/* "$SPOOLING_OUTPUT_DIR/oblivion/"
line

