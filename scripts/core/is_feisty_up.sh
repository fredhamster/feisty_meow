#!/bin/bash

# tells the caller if the feisty_meow scripts have been initialized
# successfully or not, as far as we can tell.
# any parameter passed will cause us to go into debugging mode.

do_debug="$1"; shift

if [ \
    -z "$FEISTY_MEOW_APEX" -o \
    -z "$CORE_VARIABLES_LOADED" -o \
    -z "$FEISTY_MEOW_LOADING_DOCK" \
  ]; then
  if [ ! -z "$do_debug" ]; then
    echo "Bailing out because a crucial Feisty Meow initialization variable is missing."
  fi
  exit 1
fi

# we got through the minimal gauntlet, so claim we're initialized.
if [ ! -z "$do_debug" ]; then
  echo "Looking good for every Feisty Meow initialization variable that we check."
fi
exit 0


