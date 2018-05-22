#!/bin/bash

function do_redeveloper()
{
  reapply_cool_permissions developer

  # anything else specific to developer?
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*redeveloper\.sh.* ]]; then
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  source "$FEISTY_MEOW_SCRIPTS/security/cool_permissionator.sh"
  exit_on_error "sourcing the permission script"
  do_redeveloper
  exit_on_error "redevelopering process"
fi

