#!/usr/bin/env bash

# a helpful script that scrapes any active game saves from wine's storage
# area into a spooling saves folder for archiving.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# copies the files for a particular game out to a spooling folder.
function copyem()
{
  game_name="$1"; shift
  source_dir="$1"; shift
  out_dir="$1"; shift

  if [ -d "$source_dir" -a $(ls "$source_dir" 2>/dev/null | wc -c) != 0 ]; then
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

# run through and copy our save files from the potentially weird locations they reside in.
# this requires a single argument, which is the top-level directory of the game storage.
function copy_all_save_games()
{
  local WINE_GOODS_DIR="$1"

  local WINE_GAMES_DIR="$WINE_GOODS_DIR/My Games"
  local SPOOLING_OUTPUT_DIR="$HOME/data/spooling_saves"

  if [ ! -d "$WINE_GAMES_DIR" ]; then
    echo "Failing to find the game save directories."
    exit 1
  fi

  if [ ! -d "$SPOOLING_OUTPUT_DIR" ]; then
    mkdir -p "$SPOOLING_OUTPUT_DIR"
    exit_on_error Creating spooling output directory.
  fi

  sep 28

  copyem "skyrim" "$WINE_GAMES_DIR/Skyrim/Saves" "$SPOOLING_OUTPUT_DIR/skyrim"

  copyem "fallout new vegas" "$WINE_GAMES_DIR/FalloutNV/Saves" "$SPOOLING_OUTPUT_DIR/fallout_new_vegas"

  copyem "fallout 3" "$WINE_GAMES_DIR/Fallout3/Saves" "$SPOOLING_OUTPUT_DIR/fallout_3"

  copyem "oblivion" "$WINE_GAMES_DIR/Oblivion/Saves" "$SPOOLING_OUTPUT_DIR/oblivion/"

  copyem "fallout 4" "$WINE_GAMES_DIR/Fallout4/Saves" "$SPOOLING_OUTPUT_DIR/fallout_4"

  copyem "witcher 3" "$WINE_GOODS_DIR/The Witcher 3/gamesaves" "$SPOOLING_OUTPUT_DIR/witcher_3"
}

# mainline of script tries out a few locations to back up.

# first try our play on linux storage.  very individualized for fred.
wine_goods_dir="$HOME/linx/wine_goods"
if [ ! -d "$wine_goods_dir" ]; then
  wine_goods_dir="/cygdrive/c/users/fred/My Documents"
fi
if [ ! -d "$wine_goods_dir" ]; then
  echo "not trying to back up any wine goods; could not find an appropriate folder."
else
  copy_all_save_games "$wine_goods_dir"
fi

# now try the main linux steam save game storage area, which is also fred specific.
# this uses a link in the home directory called steam_goods which is connected to
# "$HOME/.steam/steam/SteamApps/compatdata/MYNUM/pfx/drive_c/users/steamuser/My Documents"
# where MYNUM is replaced with one's steam ID number.
wine_goods_dir="$HOME/linx/steam_goods"
if [ ! -d "$wine_goods_dir" ]; then
  echo "not trying to back up any steam goods; could not find an appropriate folder."
else
  copy_all_save_games "$wine_goods_dir"
fi


