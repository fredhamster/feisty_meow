#!/bin/bash

prune_dir="$1"
if [ -z "$prune_dir" ]; then
  prune_dir="$(pwd)"
fi
pushd "$prune_dir"
echo cleaning git in directory $(pwd)
git fsck --full
git gc --prune=today --aggressive
git repack
popd
