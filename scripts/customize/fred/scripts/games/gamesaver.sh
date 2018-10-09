#!/bin/bash

# a helpful script that scrapes any active game saves from wine's storage
# area into a spooling saves folder for archiving.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

WINE_GOODS_DIR="$HOME/wine_goods"
if [ ! -d "$WINE_GOODS_DIR" ]; then
  WINE_GOODS_DIR="/cygdrive/c/users/fred/My Documents"
fi

WINE_GAMES_DIR="$WINE_GOODS_DIR/My Games"
SPOOLING_OUTPUT_DIR="$HOME/data/spooling_saves"

if [ ! -d "$WINE_GAMES_DIR" ]; then
  echo "Failing to find the game save directories."
  exit 1
fi

if [ ! -d "$SPOOLING_OUTPUT_DIR" ]; then
  mkdir -p "$SPOOLING_OUTPUT_DIR"
  exit_on_error Creating spooling output directory.
fi

# copies the files for a particular game out to a spooling folder.
function copyem()
{
  game_name="$1"; shift
  source_dir="$1"; shift
  out_dir="$1"; shift

  if [ -d "$source_dir" ]; then
    echo $game_name
    if [ ! -d "$out_dir" ]; then
      mkdir -p "$out_dir"
      exit_on_error "Creating storage dir: $out_dir"
    fi
    netcp "$source_dir"/* "$out_dir"/
    sep 28
  fi
}

##############

# now run through and copy our save files from the potentially weird locations
# they reside in.

sep 28

copyem "skyrim" "$WINE_GAMES_DIR/Skyrim/Saves" "$SPOOLING_OUTPUT_DIR/skyrim"

copyem "fallout new vegas" "$WINE_GAMES_DIR/FalloutNV/Saves" "$SPOOLING_OUTPUT_DIR/fallout_new_vegas"

copyem "fallout 3" "$WINE_GAMES_DIR/Fallout3/Saves" "$SPOOLING_OUTPUT_DIR/fallout_3"

copyem "oblivion" "$WINE_GAMES_DIR/Oblivion/Saves" "$SPOOLING_OUTPUT_DIR/oblivion/"

copyem "fallout 4" "$WINE_GAMES_DIR/Fallout4/Saves" "$SPOOLING_OUTPUT_DIR/fallout_4"

copyem "witcher 3" "$WINE_GOODS_DIR/The Witcher 3/gamesaves" "$SPOOLING_OUTPUT_DIR/witcher_3"


