#!/bin/bash

function do_redirt()
{
  reapply_cool_permissions dirt

  # anything else specific to dirt?
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*redirt\.sh.* ]]; then
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  source "$FEISTY_MEOW_SCRIPTS/security/cool_permissionator.sh"
  exit_on_error "sourcing the permission script"
  do_redirt
  continue_on_error "redirting process"
fi

