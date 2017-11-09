#!/bin/bash

# compresses the git archive in the folder specified.

prune_dir="$1"
if [ -z "$prune_dir" ]; then
  prune_dir="$(pwd)"
fi
pushd "$prune_dir"
echo "cleaning git repo in directory $(pwd)"
git fsck --full
test_or_die "git fsck"
git gc --prune=today --aggressive
test_or_die "git gc"
git repack
test_or_die "git repack"
popd
