#!/usr/bin/env bash

# got really tired of seeing this as a big long single line in jenkins, plus
# it kept breaking and was a huge pain to edit.  so now it's in a much more
# readable file.
# the only parameter is the path to the feisty meow codebase being built and
# tested.

feisty_path="$1"; shift

if [ -z "$feisty_path" ]; then
  echo This script requires the path to the feisty meow codebase under test.
  exit 1
fi

export RUN_ALL_TESTS=true
# clean up any old home storage paths.
\rm -rf "$feisty_path/home_store".*
# set home folder to a new home_store.random folder, for anything that
# feisty meow needs to store under $HOME.
export HOME="$(mktemp -d "$feisty_path/home_store.XXXXXX")"
cd "$feisty_path"
export FEISTY_MEOW_APEX="$(\pwd)"
export FEISTY_MEOW_SCRIPTS=$FEISTY_MEOW_APEX/scripts
bash "$feisty_path/scripts/core/reconfigure_feisty_meow.sh"
source "$feisty_path/scripts/core/launch_feisty_meow.sh"
var FEISTY_MEOW_APEX FEISTY_MEOW_SCRIPTS
bash "$feisty_path/scripts/generator/produce_feisty_meow.sh"

