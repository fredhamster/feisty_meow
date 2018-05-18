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
  test_or_die "sourcing the feisty meow environment"
  source "$FEISTY_MEOW_SCRIPTS/security/cool_permissionator.sh"
  test_or_die "sourcing the permission script"
  do_redeveloper
  test_or_die "redevelopering process"
fi

