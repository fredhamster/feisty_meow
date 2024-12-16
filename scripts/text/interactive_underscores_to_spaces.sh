#!/usr/bin/env bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

while true; do
  sep 14 
  read line || break
  echo "< $line"
  echo "> $line" | sed -e 's/_/ /g'
done

