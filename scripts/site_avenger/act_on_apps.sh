#!/bin/bash

# run a command on all of the existing apps folders, but only if they appear to have site avenger projects in them.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# fix the app name for our call to the act_on_tree script.
export APP_NAME="$(basename $0 .sh)"

act_on_tree -f avenger5 -d ~/apps "${@}"


