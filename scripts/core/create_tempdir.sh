#!/bin/bash

# create_tempdir:
#
# This creates a temporary directory for transient files if such a directory
# does not already exist.

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo creating temporary directory...; fi

if [ -z "$TMP" ]; then
  # main declaration of the transients area.
  export TMP=$HOME/.tmp
fi

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

if [ ! -d "$TMP" ]; then
  mkdir -p $TMP
  chown $USER $TMP
  if [ $? -ne 0 ]; then
    echo "failed to chown $TMP to user's ownership."
  fi
  log_feisty_meow_event "created transient area \"$TMP\" for $USER on $(date_stringer)." 
fi

# set other temporary variables to the same place as TMP.
export TEMP=$TMP

# Make sure no one else is playing around in the temporary directory.
chmod 700 $TMP
continue_on_error chmodding to secure temporary directory.

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo done creating temporary directory....; fi

