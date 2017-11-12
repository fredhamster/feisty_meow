#!/bin/bash

# Author: Chris Koeritz

# this script adds the feisty inits code to .bashrc, if we think it has not yet been added.

# auto-locate the feisty meow scripts, since they supposedly are not enabled yet.
export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
# normalize the path we want to cobble together.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../.." && \pwd )"

echo calculated apex as $FEISTY_MEOW_APEX

do_root="$1"; shift

if [ -f "$HOME/.bashrc" ] && grep -q "launch_feisty_meow.sh" "$HOME/.bashrc"; then
  # the stanza for loading feisty meow already seems to be present.
  echo "Feisty Meow already seems to be configured in '~/.bashrc'."
else
  # check for the --root flag to see if they're trying to get the root version of inits.
  if [ "$do_root" != "--root" ]; then
    # stuff the "normal user" init file into .bashrc.  not appropriate for root.
    # this is the easy and quick start script for most folks.
    cat $FEISTY_MEOW_APEX/infobase/feisty_inits/dot.bashrc-normal-user |
      sed -e \
        "s?FEISTY_MEOW_APEX=\".*\"?FEISTY_MEOW_APEX=\"$FEISTY_MEOW_APEX\"?" \
        >> "$HOME/.bashrc"
    echo "Feisty Meow is now configured in '~/.bashrc' for standard users."
  else
    # stuff the root user init file into .bashrc.  this one doesn't automatically load
    # feisty meow.  instead, it provides a fredme macro to load the feisty meow scripts.
    # fredme comes from the main author being fred t. hamster.  we have since added a
    # feistyme macro too, to be less personalized...
    cat $FEISTY_MEOW_APEX/infobase/feisty_inits/dot.bashrc-root |
      sed -e \
        "s?FEISTY_MEOW_APEX=\".*\"?FEISTY_MEOW_APEX=\"$FEISTY_MEOW_APEX\"?" \
        >> "$HOME/.bashrc"
    echo "Feisty Meow is now configured in '~/.bashrc' for the root user."
  fi
fi

