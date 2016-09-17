#!/bin/bash

# checkin: checks in all the folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

echo "Committing repositories at: $(date)"

if [ "$OS" != "Windows_NT" ]; then
  # first get individual folders.
  checkin_list $HOME /usr/local
else
  checkin_list $HOME c:/ d:/ e:/ 
fi

