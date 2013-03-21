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
certfile="$1"; shift
if [ -z "$certfile" ]; then
  certfile=$HOME/.ssh/id_dsa_sourceforge
fi

pushd "$dir"
git fetch upstream
git merge upstream/master
git push -i "$certfile" origin master
popd
