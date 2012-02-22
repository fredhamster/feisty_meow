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

if [ -z "$FEISTY_MEOW_GENERATED" ]; then
  # FEISTY_MEOW_GENERATED is where the generated files yeti uses are located.
  # this is our single entry point we can use without knowing any variables
  # yet in the initialization process.
  export FEISTY_MEOW_GENERATED="$HOME/.zz_auto_gen"
#hmmm: the above is kind of a constant.  that's not so great.

  # make sure our main variables are established.
  GENERATED_FEISTY_MEOW_VARIABLES="$FEISTY_MEOW_GENERATED/fmc_variables.sh"
  if [ ! -f "$GENERATED_FEISTY_MEOW_VARIABLES" ]; then
    echo -e '\n\n'
    echo "The yeti scripts need to be initialized via the bootstrap process, e.g.:"
    echo "  bash $HOME/feisty_meow/scripts/core/bootstrap_shells.sh"
    echo -e '\n\n'
  fi

  ##############

  # pull in our generated variables that are the minimal set we need to find
  # the rest of our resources.
  source "$GENERATED_FEISTY_MEOW_VARIABLES"

  # Set up the temporary directory.
  source $FEISTY_MEOW_SCRIPTS/core/create_tempdir.sh

  ##############

  # load the larger body of standard feisty meow variables into the environment.
  source $FEISTY_MEOW_SCRIPTS/core/variables.sh

fi

# check hash table before searching path.
shopt -s checkhash
# don't check path for sourced files.
shopt -u sourcepath

##############

if [ -z "$LIGHTWEIGHT_INIT" ]; then
  # perform the bulkier parts of the login and initialization.

  if [ ! -z "$SHELL_DEBUG" ]; then echo heavyweight login begins...; fi

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

