#!/bin/bash

#source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# cleans up the ownership for all my files and dirs.
function refred()
{
  # first build a list of dirs based on their location in /home/archives.
  local arch_builder="archons basement codebarn games imaginations musix pooling prewar_toaster stuffing toaster walrus"
  local ARCHIVE_HOME=/home/archives
  local dirname
  local arch_addin
  for dirname in $arch_builder; do
    arch_addin+="$ARCHIVE_HOME/$dirname "
  done
#echo arch addin now is: $arch_addin

  # iterate across the list of dirs we want fred to own and change their ownership.
  for dirname in /home/fred /usr/local/fred /home/games $arch_addin; do
    if [ -d "$dirname" ]; then
      echo "refred on '$dirname'"
      sudo chown -R fred:fred $dirname
    fi
  done

  # special case for archives directory.
  if [ -d /z/stuffing -o -L /z/stuffing ]; then
    chmod g+rx,o+rx /z
    chmod g+rx,o-rwx /z/stuffing
    pushd /z/stuffing &>/dev/null
    if [ -d archives -o -L archives ]; then group_perm archives; fi
    popd &>/dev/null
  fi

  # make the logs readable by normal humans.
  sudo bash $FEISTY_MEOW_SCRIPTS/security/normal_perm.sh /var/log
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*refred\.sh.* ]]; then
  THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"
  source "$THISDIR/../../core/launch_feisty_meow.sh"
  refred
fi

