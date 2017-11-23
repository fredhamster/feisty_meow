#!/bin/bash

# got really tired of seeing this as a big long single line in jenkins, plus
# it kept breaking and was a huge pain to edit.  so now it's in a much more
# readable file.
# the only parameter is the path to the feisty meow codebase being built and
# tested.

feisty_path="$1"; shift

export RUN_ALL_TESTS=true
export HOME="$(mktemp -d "$feisty_path/home_store.XXXXXX")"
cd "$feisty_path"
bash "$feisty_path/scripts/core/reconfigure_feisty_meow.sh"
source "$feisty_path/scripts/core/launch_feisty_meow.sh"
echo after probably hosed launch
var FEISTY_MEOW_APEX FEISTY_MEOW_SCRIPTS
bash "$feisty_path/scripts/generator/produce_feisty_meow.sh"

