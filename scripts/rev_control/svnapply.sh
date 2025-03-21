#!/usr/bin/env bash
#
# Applies arbitrary commands to any svn status.
#
# For example, this shows any files that are in the working folder but are not part of the svn repository:
#    svnapply \? echo
#
# This deletes all files that are not checked into svn (escape the ? from the shell):
#    svnapply \? rm
#
# List all conflicted files:
#    svnapply C ls -l
#
# found on web at:
# http://stackoverflow.com/questions/160104/how-do-you-add-all-untracked-files-in-svn-something-like-git-add-i
#

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

save_terminal_title

PATTERN="$1"; shift

svn st | egrep "^\\${PATTERN}[ ]+" | \
  sed -e "s|^\\${PATTERN}[ ]*||" | \
  sed -e "s|\\\\|/|g" | \
  xargs -i "$@" '{}'

restore_terminal_title

