#!/bin/bash

THIS_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
pushd $THIS_FOLDER

# does this help?
export DEBUG=true

./bin/cake server -p 8766

popd


