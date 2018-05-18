#!/bin/bash

# a library file for redoing file ownership and permissions as we feel is
# appropriate.  this approach is a little bit specific to our way of doing
# things, but it does handle a lot of important fixes everyone would want,
# like making ~/.ssh really secure.

# cleans up the ownership and permissions for all of the important files and dirs.
function reapply_cool_permissions()
{
  local cooluser="$1"; shift

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

  # fix some permissions for important security considerations.
  if [ -d $HOME/.ssh ]; then
    harsh_perm $HOME/.ssh
  fi

#hmmm: consider adding feisty meow apex to the dirname list below.

  # iterate across the list of dirs we want cooluser to own and change their ownership.
  for dirname in $HOME \
        $DEFAULT_FEISTYMEOW_ORG_DIR \
        /usr/local/${cooluser} \
        /home/games \
        $arch_addin; do
    if [ -d "$dirname" ]; then
      echo "revising ownership on '$dirname'"
      sudo chown -R ${cooluser}:${cooluser} "$dirname"
      test_or_die "chowning for ${cooluser}: $dirname"
    fi
  done

  # special case for archives directory in stuffing.
  if [ -d /z/stuffing -o -L /z/stuffing ]; then
    sudo chown ${cooluser}:${cooluser} /z/
    test_or_die "chowning /z for ${cooluser}"
    sudo chmod g+rx,o+rx /z
    test_or_die "chmodding /z/ for ${cooluser}"
    sudo chown ${cooluser}:${cooluser} /z/stuffing/
    test_or_die "chowning /z/stuffing for ${cooluser}"
    sudo chmod g+rx,o-rwx /z/stuffing
    test_or_die "chmodding /z/stuffing for ${cooluser}"
    pushd /z/stuffing &>/dev/null
    if [ -d archives -o -L archives ]; then
      sudo chown ${cooluser}:${cooluser} archives/
      test_or_die "chowning /z/stuffing/archives for ${cooluser}"
      sudo chmod -R g+rwx archives
      test_or_die "chmodding /z/stuffing/archives for ${cooluser}"
    fi
    popd &>/dev/null
  fi

  # make the log files readable by normal humans.
  sudo bash $FEISTY_MEOW_SCRIPTS/security/normal_perm.sh /var/log
  test_or_die "setting normal perms on /var/log"
}

# this block should execute when the script is actually run, rather
# than when it's just being sourced.

# this runs the cool permission applier on the current user.
if [[ $0 =~ .*cool_permissionator\.sh.* ]]; then
echo A
  THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"
echo B
  export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"
echo B.2
  source "$THISDIR/../core/launch_feisty_meow.sh"
  test_or_die "sourcing the feisty meow launcher"
echo C
  reapply_cool_permissions $(logname)
  test_or_die "reapplying cool permissions on $(logname)"
echo D
fi

