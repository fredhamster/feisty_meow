#!/bin/bash
# Hamster Login Profile.
#
# This file takes the place of .profile or other initialization scripts.

export SHELL_DEBUG=true
  # this variable causes the scripts that listen to it to print more information
  # when they run.

####fault--repeated code from bootstrap.  isolate to shared location.
# FEISTY_MEOW_GENERATED is where the generated files yeti uses are located.
export FEISTY_MEOW_GENERATED="$HOME/.zz_auto_gen"
if [ ! -z "$WINDIR" -o ! -z "$windir" ]; then
  # assume they are using windoze.
  export FEISTY_MEOW_GENERATED="$TMP/zz_auto_gen"
fi

# make sure our main variables are established.
GENERATED_FEISTY_MEOW_VARIABLES="$FEISTY_MEOW_GENERATED/feisty_meow_variables.sh"
if [ ! -f "$GENERATED_FEISTY_MEOW_VARIABLES" ]; then
  echo -e '\n\n'
  echo "The yeti scripts need to be initialized via the bootstrap process, e.g.:"
  echo "  bash $HOME/feisty_meow/scripts/core/bootstrap_shells.sh"
  echo -e '\n\n'
fi

# pull in our variable set.
source "$GENERATED_FEISTY_MEOW_VARIABLES"

# define a default name, if one wasn't already set.
if [ -z "$NAME" ]; then
  export NAME='Unset Q. Namington, Fixley Your Name III'
fi

# check if this is dos/windows.
if [ "$OS" == "Windows_NT" ]; then
  if [ -z "$HOME" ]; then
    # set a default that might not be appropriate for everyone, but should
    # still work.
    export HOME=/c/home
  fi
  if [ ! -d "$HOME" ]; then mkdir $HOME; fi
##  export FEISTY_MEOW_GENERATED=$TMP/zz_auto_gen
fi

if [ -z "$LIGHTWEIGHT_INIT" ]; then
  # perform the bulk of the login.
  source $FEISTY_MEOW_SCRIPTS/core/unix_login.sh
else
  # this is the lightweight login that just wants variables set.
  source $FEISTY_MEOW_SCRIPTS/core/lightweight_unix_login.sh
fi

