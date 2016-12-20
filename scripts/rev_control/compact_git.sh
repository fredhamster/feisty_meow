#!/bin/bash

prune_dir="$1"
if [ -z "$prune_dir" ]; then
  prune_dir="$(pwd)"
fi
pushd "$prune_dir"
echo cleaning git in directory $(pwd)
git fsck --full
check_if_failed "git fsck"
git gc --prune=today --aggressive
check_if_failed "git gc"
git repack
check_if_failed "git repack"
popd
