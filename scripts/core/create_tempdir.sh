#!/bin/bash

# create_tempdir:
#
# This creates a temporary directory for transient files if such a directory
# does not already exist.

if [ ! -z "$SHELL_DEBUG" ]; then echo creating temporary directory...; fi

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

if [ -z "$TMP" ]; then
  # main declaration of the transients area.
  export TMP=$HOME/.tmp
fi

LOG_FILE=$TMP/zz_transients.log
  # log file for this script.

word=Verified
if [ ! -d "$TMP" ]; then
  mkdir $TMP
  word=Created
  chown $USER $TMP
  if [ $? -ne 0 ]; then
    echo "failed to chown $TMP to user's ownership."
  fi
fi
if [ -z "$LIGHTWEIGHT_INIT" ]; then
  echo "$word transient area \"$TMP\" for $USER on $(date_stringer)." >>$LOG_FILE
fi

# set other temporary variables to the same place as TMP.
export TEMP=$TMP

# Make sure no one else is playing around in the temporary directory.
chmod 700 $TMP

if [ ! -z "$SHELL_DEBUG" ]; then echo done creating temporary directory....; fi

