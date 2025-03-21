#!/usr/bin/env bash

# checkin: checks in all the folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

echo "committing repositories at: $(date)"

FULL_LIST=" $(dirname $FEISTY_MEOW_APEX) $HOME "
if [ "$OS" == "Windows_NT" ]; then
  FULL_LIST+=" c:/ d:/ e:/ "
fi

checkin_list $FULL_LIST
exit_on_error "revision control check-in of list: $FULL_LIST"

echo

##############

# regenerate the scripts after the check-in, since an update during check-in
# could mean we have a modified version of feisty meow is present.
regenerate

##############

restore_terminal_title

