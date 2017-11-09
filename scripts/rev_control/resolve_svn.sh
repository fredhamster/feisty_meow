#!/bin/bash

# resolves a tree conflict by accepting the "working" version,
# which effectively makes your current change the accepted one.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

filename="$1"; shift

if [ -z "$filename" ]; then
  echo "This script needs a filename to operate on."
  exit 1
fi

svn resolve --accept=working "$filename"
test_or_die "resolving tree conflict by accepting the working directory as the right one"


