#!/bin/bash
##############
# Name   : gpg-daemon-launcher
# Author : Chris Koeritz
# Rights : Copyright (C) 2012-$now by Feisty Meow Concerns, Ltd.
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############

# starts up the gpg-agent, but only if it's not already running.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

if [ -z "$(psa gpg-agent)" ]; then
  gpg-agent --daemon --enable-ssh-support --write-env-file "${HOME}/.gpg-agent-info" &>$TMP/zz_gpg-agent-daemon.log
fi



