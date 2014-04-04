#!/bin/bash

if [ "$OPERATING_SYSTEM" = "WIN32" ]; then
  source $FEISTY_MEOW_SCRIPTS/core/functions.sh

  TMP="$(dos_to_unix_path "$TMP")"
fi

#echo TMP is $TMP 

/usr/bin/svn $@

