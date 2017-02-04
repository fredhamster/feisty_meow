#!/bin/bash

# checkin: checks in all the folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

echo "committing repositories at: $(date)"

if [ "$OS" != "Windows_NT" ]; then
  # first get individual folders.
  checkin_list $HOME /usr/local
else
  checkin_list $HOME c:/ d:/ e:/ 
fi

##############

# regenerate the scripts after checking in, since this could mean a modified version
# of feisty meow is present.
regenerate

##############

