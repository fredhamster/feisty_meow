#!/usr/bin/env bash

# compresses the git archive in the folder specified.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

prune_dir="$1"
if [ -z "$prune_dir" ]; then
  prune_dir="$(pwd)"
fi
pushd "$prune_dir" &>/dev/null
exit_on_error "changing to directory: $prune_dir"

echo "cleaning git repo in directory $(pwd)"

git fsck --full
exit_on_error "git fsck"

git gc --prune=today --aggressive
exit_on_error "git gc"

git repack
exit_on_error "git repack"

popd &>/dev/null

restore_terminal_title

