#!/bin/bash

#
# Unix Start up information and personal environment for bash.
#

if [ ! -z "$SHELL_DEBUG" ]; then echo unix login begins...; fi

GENERADIR=$(echo $GENERADIR | sed -e 's/\\/\//g')
YETI_SCRIPTS="$(echo $YETI_SCRIPTS | sed -e 's/\\/\//g')"
SHELLDIR="$YETI_SCRIPTS"

# Set up all of the environment's system variables.  This is the first step
# in the majestic plan we have for this shell's initialization.
source $YETI_SCRIPTS/core/variables.sh

# Also set up the temporary directory...
source $YETI_SCRIPTS/core/create_tempdir.sh

# ulimit and umask.  umask sets a permission mask for all file
# creations.  The mask shown here disallows writing by the "group" and
# "others" categories of users.  ulimit sets the user limits.  the core
# file size is set to zero.
umask 022
ulimit -c 0

# The second part of this sweeping two-part inauguration process is to set
# up the aliases for the shell, but only if they are not already set.  The
# only alias we know of that's specific to our set is used.
alias lsd >/dev/null 2>/dev/null  # see if the 'x' alias exists.
if [ $? -ne 0 ]; then
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo the aliases were missing, now they are added...
  fi
  source $GENERADIR/aliases.sh
fi

# allow connections to our x server from the local host.
if [ ! -z "$DISPLAY" ]; then
  if [ ! -z "$(echo "$OS_TYPE" | grep -i darwin)" ]; then
    if [ ! -z "$SHELL_DEBUG" ]; then echo Enabling localhost X connections...; fi
    xhost + localhost >/dev/null 2>&1
  fi
fi

# a minor tickle of the title of the terminal, in case there is one.
bash $YETI_SCRIPTS/tty/label_terminal_with_infos.sh

if [ ! -z "$SHELL_DEBUG" ]; then echo unix login ends....; fi

