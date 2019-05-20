#!/bin/bash

function do_refred()
{
  reapply_cool_permissions fred

  # anything else specific to fred?
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*refred\.sh.* ]]; then
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  source "$FEISTY_MEOW_SCRIPTS/security/cool_permissionator.sh"
  exit_on_error "sourcing the permission script"
  do_refred
  continue_on_error "refredding process"
fi

