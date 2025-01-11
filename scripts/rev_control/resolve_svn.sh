#!/usr/bin/env bash

# resolves a tree conflict by accepting the "working" version,
# which effectively makes your current change the accepted one.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

filename="$1"; shift

if [ -z "$filename" ]; then
  echo "This script needs a filename to operate on."
  exit 1
fi

svn resolve --accept=working "$filename"
exit_on_error "resolving tree conflict by accepting the working directory as the right one"

restore_terminal_title

