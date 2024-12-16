#!/usr/bin/env bash

THIS_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
pushd $THIS_FOLDER

# does this help?
export DEBUG=true

./bin/cake server -H 192.241.191.154 -p 12738

popd


