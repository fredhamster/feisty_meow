#!/bin/bash

##############

# Fred Hamster's Feisty Meow Concerns Ltd. Startup Profile.
#
# This file is useful within .profile or other initialization scripts.
#
# Author: Chris Koeritz

##############

# SHELL_DEBUG: if this variable is non-empty, then it causes the feisty meow
# scripts to print more diagnostic information when they run.  not all
# scripts support this, but the core ones do.

#export SHELL_DEBUG=true

##############

# patch the user variable if we were launched by one of our cron jobs.
if [ -z "$USER" -a ! -z "$CRONUSER" ]; then
  export USER="$CRONUSER"
fi

##############

export ERROR_OCCURRED=
  # there have been no errors to start with, at least.  we will set this
  # to non-empty if something bad happens.

if [ -z "$FEISTY_MEOW_LOADING_DOCK" ]; then
  # FEISTY_MEOW_LOADING_DOCK is where the generated files are located.
  # this is our single entry point we can use without knowing any variables
  # yet in the initialization process.
  export FEISTY_MEOW_LOADING_DOCK="$HOME/.zz_feisty_loading"
#hmmm: the above is kind of a constant.  that's not so great.

  # make sure our main variables are established.
  FEISTY_MEOW_VARIABLES_LOADING_FILE="$FEISTY_MEOW_LOADING_DOCK/fmc_variables.sh"
  if [ ! -f "$FEISTY_MEOW_VARIABLES_LOADING_FILE" ]; then
    echo -e '\n\n'
    echo "Feisty meow scripts need initialization via the bootstrap process, e.g.:"
    echo "  bash $HOME/feisty_meow/scripts/core/reconfigure_feisty_meow.sh"
    echo -e '\n\n'
    ERROR_OCCURRED=true
  fi

  ##############

  # pull in our generated variables that are the minimal set we need to find
  # the rest of our resources.
  source "$FEISTY_MEOW_VARIABLES_LOADING_FILE"

  # Set up the temporary directory.
  source "$FEISTY_MEOW_SCRIPTS/core/create_tempdir.sh"

fi

##############

# load the larger body of standard feisty meow variables into the environment.
# we actually want this to always run also; it will decide what variables need
# to be set again.
source "$FEISTY_MEOW_SCRIPTS/core/variables.sh"

##############
  
# include helpful functions.  we do this every time rather than making it part
# of variable initialization, because functions cannot be exported to
# sub-shells in bash (much like aliases cannot, to our infinite chagrin after
# having migrated from korn shell...).
source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
  
# load some helper methods for the terminal which we'll use below.
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

##############
  
# check hash table before searching path.
shopt -s checkhash
# don't check path for sourced files.
shopt -u sourcepath
# ignore duplicate lines.
HISTCONTROL=ignoredups
# append to the history file.
shopt -s histappend
# automatically update window size if needed.
shopt -s checkwinsize

##############

# make history writes immediate to avoid losing history if bash is zapped.
echo $PROMPT_COMMAND | grep -q history
if [ $? -ne 0 ]; then
  # we only change the prompt command if we think it hasn't already been done.
  export PROMPT_COMMAND="history -a;$PROMPT_COMMAND"
fi

##############

# perform the bulkier parts of the initialization process.

if [ ! -z "$SHELL_DEBUG" ]; then echo "heavyweight init begins..."; fi

# set up the aliases for the shell, but only if they are not already set.
type CORE_ALIASES_LOADED &>/dev/null
if [ $? -ne 0 ]; then
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "the aliases were missing, now they are being added..."
  fi
  source "$FEISTY_MEOW_LOADING_DOCK/fmc_core_and_custom_aliases.sh"
fi

#echo before the new labelling, terminal titles have:
#show_terminal_titles

# a minor tickle of the title of the terminal, unless we already have some history.
label_terminal_with_info

if [ ! -z "$SHELL_DEBUG" ]; then echo "heavyweight init is done."; fi

if [ -z "$ERROR_OCCURRED" ]; then
  # set a sentinel variable to say we loaded the feisty meow environment.
  export FEISTY_MEOW_SCRIPTS_LOADED=true
fi

