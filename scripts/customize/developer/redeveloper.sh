#!/bin/bash

#source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# cleans up the ownership for all my files and dirs.
function redeveloper()
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

  # special case that makes our software hierarchy folder, if it doesn't exist.
  # everything else is only re-permed if it exists.
  if [ ! -d "$DEFAULT_FEISTYMEOW_ORG_DIR" ]; then
    sudo mkdir "$DEFAULT_FEISTYMEOW_ORG_DIR"
    test_or_die "making directory: $DEFAULT_FEISTYMEOW_ORG_DIR"
  fi

  # iterate across the list of dirs we want developer to own and change their ownership.
  for dirname in /home/developer $DEFAULT_FEISTYMEOW_ORG_DIR /usr/local/developer /home/games $arch_addin; do
    if [ -d "$dirname" ]; then
      echo "redeveloper on '$dirname'"
      sudo chown -R developer:developer "$dirname"
      test_or_die "chowning for developer: $dirname"
    fi
  done

  # special case for archives directory.
  if [ -d /z/stuffing -o -L /z/stuffing ]; then
    sudo chown developer:developer /z
    test_or_die "chowning /z for developer"
    sudo chmod g+rx,o+rx /z
    test_or_die "chmodding /z/ for developer"
    sudo chown developer:developer /z/stuffing
    test_or_die "chowning /z/stuffing for developer"
    sudo chmod g+rx,o-rwx /z/stuffing
    test_or_die "chmodding /z/stuffing for developer"
    pushd /z/stuffing &>/dev/null
    if [ -d archives -o -L archives ]; then
      sudo chown developer:developer archives
      test_or_die "chowning /z/stuffing/archives for developer"
      sudo chmod -R g+rwx archives
      test_or_die "chmodding /z/stuffing/archives for developer"
    fi
    popd &>/dev/null
  fi

  # make the logs readable by normal humans.
  sudo bash $FEISTY_MEOW_SCRIPTS/security/normal_perm.sh /var/log
  test_or_die "setting normal perms on /var/log"
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*redeveloper\.sh.* ]]; then
  THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"
  source "$THISDIR/../../core/launch_feisty_meow.sh"
  test_or_die "sourcing the feisty meow launcher"
  redeveloper
  test_or_die "redeveloperding process"
fi

