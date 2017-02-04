#!/bin/bash

# cleans up the ownership for all my files.
function refred()
{
  sudo chown -R fred:fred /home/fred /home/games /home/archives /usr/local/fred
  sudo bash $FEISTY_MEOW_SCRIPTS/security/normal_perm.sh /var/log
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*refred\.sh.* ]]; then
  THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"
  export LIGHTWEIGHT_INIT=true
  source "$THISDIR/../../core/launch_feisty_meow.sh"
  refred
fi

