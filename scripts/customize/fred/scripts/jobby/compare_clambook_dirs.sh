#!/usr/bin/env bash

# a little helper script for clambook,
# which is uva's mac and which doesn't share syncthing.
# we can easily see what stuff got accidentally put on
# the /z folders with this, and make sure anything new
# actually gets back to the true sources of these files.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

DATA_SOURCE_HOST="curie"

function compare_to_curie()
{
  dir="$1"; shift
  s
  echo
  echo "comparing $dir"
  echo
  echo "'<' will be remote, '>' will be local."
  echo
  compare_dirs "fred@${DATA_SOURCE_HOST}:/z/$dir" "/z/$dir"
  retval=$?
  s
  echo
  return $retval
}

compare_to_curie musix
compare_to_curie walrus


