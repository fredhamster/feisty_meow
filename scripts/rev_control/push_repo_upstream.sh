#!/bin/bash

# this script updates a "relay" repository (let us call it B) that is used
# to mirror things from repository A (the source) into another repository C
# (the target).
# this is useful, for example, to maintain one's own master git archive for
# a codebase, but also push updates for that codebase into a sourceforge git
# repository.
#
# rats: how did i set up that archive?
#       we need to have those steps someplace.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

# this file needs to have our sourceforge password in it.
PASSWORD_FILE="$HOME/.secrets/sourceforge_password"

if [ ! -f "$PASSWORD_FILE" ]; then
  echo "This script requires a password stored in the file:"
  echo "  $PASSWORD_FILE"
  exit 1
fi

pushd "$dir"
git fetch upstream
git merge upstream/master
unset GIT_SSH
git push origin master <"$PASSWORD_FILE"
popd


