#!/bin/bash

# branchy: lists the branches active on all of the folders present in the
# REPOSITORY_LIST variable or in paths provided on the command line.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

echo "showing repository branches at: $(date)"

FULL_LIST="$@"

# only add the standard feisty meow repo list items if we were given no args on command line.
#hmmm: below seems to be oddly just major locations; is the repo list all relative names?
if [ -z "$FULL_LIST" ]; then
  FULL_LIST=" $(dirname $FEISTY_MEOW_APEX) $HOME "
  if [ "$OS" == "Windows_NT" ]; then
    FULL_LIST+=" c:/ d:/ e:/ "
  fi
fi

show_active_branch $FULL_LIST
exit_on_error "showing branches: $FULL_LIST"

echo

##############

restore_terminal_title
