#!/bin/bash
#
# Applies arbitrary commands to any svn status. e.g.
#
# Delete all non-svn files (escape the ? from the shell):
# svnapply \? rm
#
# List all conflicted files:
# svnapply C ls -l
#
# found on web at: http://stackoverflow.com/questions/160104/how-do-you-add-all-untracked-files-in-svn-something-like-git-add-i
#

PATTERN="$1"; shift

svn st | egrep "^\\${PATTERN}[ ]+" | \
  sed -e "s|^\\${PATTERN}[ ]*||" | \
  sed -e "s|\\\\|/|g" | \
  xargs -i "$@" '{}'

