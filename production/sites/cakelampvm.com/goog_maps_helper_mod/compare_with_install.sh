#!/bin/bash


if [ ! -d "$HOME/apps/mapsdemo/avenger5" ]; then
  echo Not seeing the mapsdemo checked out man.
  exit 1
fi

meld $(find ~/apps/mapsdemo/avenger5/ -iname goog*map*helper.php) ./Goog*Map*Help*p


