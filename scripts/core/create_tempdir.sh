#!/bin/bash

# create_tempdir:
#
# This creates a temporary directory for transient files if such a directory
# does not already exist.

source $FEISTY_MEOW_SCRIPTS/core/date_stringer.sh

if [ ! -z "$SHELL_DEBUG" ]; then echo creating temporary directory...; fi

if [ -z "$TMP" ]; then
  export TMP=$HOME/.tmp
    # main declaration of the transients area.
  if [ "$OS" = "Windows_NT" ]; then
    export TMP=c:/tmp
  fi
fi

LOG_FILE=$TMP/zz_transients.log
  # log file for this script.

word=Verified
if [ ! -d "$TMP" ]; then
  mkdir $TMP
  word=Created
fi
if [ -z "$LIGHTWEIGHT_INIT" ]; then
  echo "$word transient area \"$TMP\" for $USER on $(date_stringer)." >>$LOG_FILE
fi

# set other temporary variables to the same place as TMP.
export TEMP=$TMP

# Make sure no one else is playing around in the temporary directory.
chmod 700 $TMP

if [ ! -z "$SHELL_DEBUG" ]; then echo done creating temporary directory....; fi

