#!/usr/bin/env bash

# windows compatible process killer for any stray processes started as
# "witcher" something.

source $FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh

procid="$( psa witcher | tail -n 1 | awk '{ print $1; }' )"

if [ ! -z "$procid" ]; then
  taskkill.exe /f /pid ${procid}
fi
