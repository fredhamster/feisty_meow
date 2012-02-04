#!/bin/bash

##############

# Fred Hamster's Feisty Meow Concerns Ltd. Startup Profile.
#
# This file is useful within .profile or other initialization scripts.
#
# Author: Chris Koeritz

##############

#export SHELL_DEBUG=true
  # this variable causes the scripts that listen to it to print more information
  # when they run.

##############

# FEISTY_MEOW_GENERATED is where the generated files yeti uses are located.
# this is our single entry point we can use without knowing any variables
# yet in the initialization process.
export FEISTY_MEOW_GENERATED="$HOME/.zz_auto_gen"
if [ ! -z "$WINDIR" -o ! -z "$windir" ]; then
  # assume they are using windoze.
  export FEISTY_MEOW_GENERATED="$TMP/zz_auto_gen"
fi

# make sure our main variables are established.
GENERATED_FEISTY_MEOW_VARIABLES="$FEISTY_MEOW_GENERATED/fmc_variables.sh"
if [ ! -f "$GENERATED_FEISTY_MEOW_VARIABLES" ]; then
  echo -e '\n\n'
  echo "The yeti scripts need to be initialized via the bootstrap process, e.g.:"
  echo "  bash $HOME/feisty_meow/scripts/core/bootstrap_shells.sh"
  echo -e '\n\n'
fi

# pull in our variable set.
source "$GENERATED_FEISTY_MEOW_VARIABLES"

##############

# Set up all of the environment's system variables.  This is the first step
# in the majestic plan we have for this shell's initialization.
source $FEISTY_MEOW_SCRIPTS/core/variables.sh

# Set up the temporary directory...
source $FEISTY_MEOW_SCRIPTS/core/create_tempdir.sh

##############

# check if this is dos/windows.
if [ "$OS" == "Windows_NT" ]; then
  if [ -z "$HOME" ]; then
    # set a default that might not be appropriate for everyone, but should
    # still work.
    export HOME=/c/home
  fi
  if [ ! -d "$HOME" ]; then mkdir $HOME; fi
fi

##############

if [ -z "$LIGHTWEIGHT_INIT" ]; then
  # perform the bulkier parts of the login and initialization.

  if [ ! -z "$SHELL_DEBUG" ]; then echo heavyweight login begins...; fi

#FEISTY_MEOW_GENERATED=$(echo $FEISTY_MEOW_GENERATED | sed -e 's/\\/\//g')
#FEISTY_MEOW_SCRIPTS="$(echo $FEISTY_MEOW_SCRIPTS | sed -e 's/\\/\//g')"
#FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_SCRIPTS"

  # set up the aliases for the shell, but only if they are not already set.
  alias regenerated &>/dev/null  # see if a crucial alias exists.
  if [ $? -ne 0 ]; then
    if [ ! -z "$SHELL_DEBUG" ]; then
      echo the aliases were missing, now they are added...
    fi
    source "$FEISTY_MEOW_GENERATED/fmc_core_and_custom_aliases.sh"
  fi

  # allow connections to our x server from the local host.
  if [ ! -z "$DISPLAY" ]; then
    if [ ! -z "$(echo "$OS_TYPE" | grep -i darwin)" ]; then
      if [ ! -z "$SHELL_DEBUG" ]; then echo Enabling localhost X connections...; fi
      xhost + localhost >/dev/null 2>&1
    fi
  fi

  # a minor tickle of the title of the terminal, in case there is one.
  bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh

  if [ ! -z "$SHELL_DEBUG" ]; then echo heavyweight login ends....; fi
fi

