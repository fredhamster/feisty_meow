#!/bin/bash

# Author: Chris Koeritz

# this script adds the feisty inits code to .bashrc, if we think it has not yet been added.

# auto-locate the feisty meow scripts, since they supposedly are not enabled yet.
export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
# normalize the path we want to cobble together.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../.." && \pwd )"

echo calculated apex as $FEISTY_MEOW_APEX

if [ -f "$HOME/.bashrc" ] && grep -q "launch_feisty_meow.sh" "$HOME/.bashrc"; then
  # the stanza for loading feisty meow already seems to be present.
  echo "Feisty Meow already seems to be configured in '~/.bashrc'."
else
  # stuff the normal user init file into .bashrc.  not appropriate for root probably, but
  # this is the easy quick start script for normal folks.
  cat $FEISTY_MEOW_APEX/feisty_inits/dot.bashrc-normal-user |
    sed -e \
      "s?FEISTY_MEOW_APEX=\".*\"?FEISTY_MEOW_APEX=\"$FEISTY_MEOW_APEX\"?" \
      >> "$HOME/.bashrc"
  echo "Feisty Meow is now configured in '~/.bashrc'."
fi

