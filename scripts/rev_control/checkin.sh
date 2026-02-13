#!/usr/bin/env bash

# checkin: checks in all the folders present in the REPOSITORY_LIST_TO_COMMIT variable,
# after updating everyone in REPOSITORY_LIST_TO_PULL.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

echo "committing repositories at: $(date)"

FULL_LIST_OUT="${REPOSITORY_LIST_TO_PULL}"
#still wrong: FULL_LIST_OUT=" $(dirname $FEISTY_MEOW_APEX) $HOME "
#ack: if [ "$OS" == "Windows_NT" ]; then
#  FULL_LIST+=" c:/ d:/ e:/ "
#fi
puff_out_list $FULL_LIST_OUT
exit_on_error "revision control puffing-out of list: $FULL_LIST_OUT"

FULL_LIST_IN="${REPOSITORY_LIST_TO_COMMIT}"
checkin_list $FULL_LIST_IN
exit_on_error "revision control check-in of list: $FULL_LIST_IN"

echo

##############

# regenerate the scripts after the check-in, since an update during check-in
# could mean we have a modified version of feisty meow is present.
regenerate

##############

restore_terminal_title

