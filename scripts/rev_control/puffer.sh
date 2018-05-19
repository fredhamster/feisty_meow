#!/bin/bash

# puffer: "puffs out" all of the folders present in the REPOSITORY_LIST
# variable, which causes the repo to be merged with its remote versions.
# this enables a clean check-in; after puffer runs, there will be no secret
# upstream changes that could mess up the git push (svn and cvs are not
# supported in this script, since they branch differently than git).

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

echo "puffing out repositories at: $(date)"
echo

FULL_LIST=" $(dirname $FEISTY_MEOW_APEX) $HOME "
if [ "$OS" == "Windows_NT" ]; then
  FULL_LIST+=" c:/ d:/ e:/ "
fi

puff_out_list $FULL_LIST
exit_on_error "puffing out list: $FULL_LIST"

##############

# regenerate the scripts after puffing out, since this could mean a modified version
# of feisty meow is present.
regenerate

##############

