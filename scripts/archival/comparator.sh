#!/bin/bash

# compares this machine's local archives against an exemplar set.

target="$1"; shift

if [ -z "$target" ]; then
  target=wildmutt
  #target=curie
fi

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/archival/general_updater.sh"

uber_archive_comparator $target

