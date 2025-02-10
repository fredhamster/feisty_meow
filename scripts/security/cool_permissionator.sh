#!/usr/bin/env bash

# a library file for redoing file ownership and permissions as we feel is
# appropriate.  this approach is a little bit specific to our way of doing
# things, but it does handle a lot of important fixes everyone would want,
# like making ~/.ssh really secure.

# cleans up the ownership and permissions for all of the important files and dirs.
function reapply_cool_permissions()
{
  local cooluser="$1"; shift

#hmmm: check for non empty name.
  local homebase="/home/$cooluser"

  # first build a list of dirs based on their location in the ARCHIVE_TOP.
  local arch_builder="archons basement codebarn games imaginations musix pooling prewar_toaster stuffing toaster walrus"
#hmmm: yeah, this is totally non-portable.  define this name specification process as a set of variables instead.
  local ARCHIVE_TOP=/home/archives
  local dirname
  local arch_addin
  for dirname in $arch_builder; do
    arch_addin+="$ARCHIVE_TOP/$dirname "
  done
#echo arch addin now is: $arch_addin

  # now another round with similar setup, to ensure we get any directories
  # that actually live out in /z but not in /home/archives.
#hmmm: bizarro.  makes the special case stuff even more unnecessary.
#      if keeping this extra step, drive the process with a list instead!!!
  ARCHIVE_TOP=/z
  for dirname in $arch_builder; do
    arch_addin+="$ARCHIVE_TOP/$dirname "
  done
#echo arch addin now is: $arch_addin

  # special case that makes our software hierarchy folder, if it doesn't exist.
  # everything else is only re-permed if it exists.
  if [ ! -d "$DEFAULT_FEISTYMEOW_ORG_DIR" ]; then
    sudo mkdir "$DEFAULT_FEISTYMEOW_ORG_DIR"
    continue_on_error "making directory: $DEFAULT_FEISTYMEOW_ORG_DIR"
  fi

  # fix some permissions for important security considerations.
  if [ -d $homebase/.ssh ]; then
    harsh_perm $homebase/.ssh
  fi

#hmmm: consider adding feisty meow apex to the dirname list below.

  # iterate across the list of dirs we want cooluser to own and change their ownership.
#hmmm: below are components of the uber list of things to fix perms on...
  for dirname in $homebase \
        $DEFAULT_FEISTYMEOW_ORG_DIR \
        /usr/local/${cooluser} \
        /home/games \
        $arch_addin; do
    if [ -d "$dirname" -o -L "$dirname" ]; then
      echo "revising ownership on '$dirname'"
      sudo chown -R ${cooluser}:${cooluser} "$dirname"
      continue_on_error "chowning '$dirname' for ${cooluser}"
      sudo chmod g+rx,o+rx "$dirname"
      continue_on_error "chmodding '$dirname' for ${cooluser}"
    fi
  done

  # make the log files readable by normal humans.
  sudo bash $FEISTY_MEOW_SCRIPTS/security/normal_perm.sh /var/log
  continue_on_error "setting normal perms on /var/log"
}

####

# this block executes when the script is actually run, rather than when it's just being sourced.

# this runs the cool permission applier on the current user.
if [[ $0 =~ .*cool_permissionator\.sh.* ]]; then
  THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"
  export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"
  source "$THISDIR/../core/launch_feisty_meow.sh"
  continue_on_error "sourcing the feisty meow launcher"
  coolio="$(sanitized_username)"
  reapply_cool_permissions "$coolio"
  continue_on_error "reapplying cool permissions on $coolio"
fi

