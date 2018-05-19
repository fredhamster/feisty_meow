#!/bin/bash

# checkin: checks in all the folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

echo "committing repositories at: $(date)"
echo

FULL_LIST=" $(dirname $FEISTY_MEOW_APEX) $HOME "
if [ "$OS" == "Windows_NT" ]; then
  FULL_LIST+=" c:/ d:/ e:/ "
fi

checkin_list $FULL_LIST
exit_on_error "checking in list: $FULL_LIST"

##############

# regenerate the scripts after checking in, since this could mean a modified version
# of feisty meow is present.
regenerate

##############

