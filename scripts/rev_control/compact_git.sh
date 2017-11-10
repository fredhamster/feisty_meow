#!/bin/bash

# compresses the git archive in the folder specified.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

prune_dir="$1"
if [ -z "$prune_dir" ]; then
  prune_dir="$(pwd)"
fi
pushd "$prune_dir" &>/dev/null
test_or_die "changing to directory: $prune_dir"

echo "cleaning git repo in directory $(pwd)"

git fsck --full
test_or_die "git fsck"

git gc --prune=today --aggressive
test_or_die "git gc"

git repack
test_or_die "git repack"

popd &>/dev/null

