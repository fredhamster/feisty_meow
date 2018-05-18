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
  test_or_die "sourcing the feisty meow environment"
  source "$FEISTY_MEOW_SCRIPTS/security/cool_permissionator.sh"
  test_or_die "sourcing the permission script"
  do_refred
  test_or_die "refredding process"
fi

