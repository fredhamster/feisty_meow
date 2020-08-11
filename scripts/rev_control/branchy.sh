#!/bin/bash

# branchy: lists the branches active on all of the folders present in the
# REPOSITORY_LIST variable or in paths provided on the command line.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

FULL_LIST="$@"

# use the current directory if no paths were provided.
if [ -z "$FULL_LIST" ]; then
  FULL_LIST="."
fi

show_active_branch $FULL_LIST
exit_on_error "showing branches: $FULL_LIST"

##############

restore_terminal_title

