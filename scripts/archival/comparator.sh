#!/bin/bash

# compares this machine's local archives against an exemplar set.

remote_place="$1"; shift
if [ -z "$remote_place" ]; then
  remote_place=wildmutt
  #remote_place=curie
fi
local_place="$1"; shift
if [ -z "$local_place" ]; then
  local_place=/z
fi

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/archival/general_updater.sh"

uber_archive_comparator $remote_place $local_place

