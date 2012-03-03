#!/bin/bash

# a wrapper for the secure shell.
# we want to fix any terminal titles that the foreign shells give us, and
# this script is our chance to do so.

#hmmm: not the slightest bit general here.
#      what about having a main key variable and a sourceforge key variable?
#      better yet, an array of site patterns and keys for those sites.

keyfile="$HOME/.ssh/id_dsa_fred"

if [ ! -z "$(echo $* | grep -i sourceforge)" ]; then
  keyfile="$HOME/.ssh/id_dsa_sourceforge"
fi

\ssh -i "$keyfile" -X $*

if [ $? -eq 0 ]; then
  # re-run the terminal labeller after coming back from ssh.
  # we check the exit value because we don't want to update this for a failed connection.
  bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
fi


