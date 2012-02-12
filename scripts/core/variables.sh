#!/bin/bash

##############

# variables script:
#   Defines the environment variables used by the personalized unix
#   environment.
# Author: Chris Koeritz

##############

if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization begins...; fi

##############

# System variables...

# OS variable records the operating system we think we found.
if [ -z "$OS" ]; then
  export OS=UNIX
fi
export IS_DARWIN=$(echo $OSTYPE | grep -i darwin)

if [ -z "$HOME" ]; then
  if [ "$OS" == "Windows_NT" ]; then
    export HOME=/c/home
    if [ ! -d $HOME ]; then
      mkdir $HOME
    fi
  fi
fi

# patch home to undo cygwin style of drive letter.
export HOME=$(echo $HOME | sed -e 's/\/cygdrive\//\//g')
#echo HOME is now $HOME

if [ "$OS" == "Windows_NT" ]; then
  export HOSTNAME=$(echo $HOSTNAME | tr A-Z a-z)
fi

# ulimit and umask.  umask sets a permission mask for all file
# creations.  The mask shown here disallows writing by the "group" and
# "others" categories of users.  ulimit sets the user limits.  the core
# file size is set to zero.
umask 022
ulimit -c 0

##############

# Directory variables...

export SCRIPT_SYSTEM=feisty_meow

#if [ -z "$FEISTY_MEOW_DIR" ]; then export FEISTY_MEOW_DIR="$HOME/$SCRIPT_SYSTEM"; fi
#if [ -z "$FEISTY_MEOW_SCRIPTS" ]; then export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_DIR/scripts"; fi
#if [ -z "$FEISTY_MEOW_SCRIPTS" ]; then export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_SCRIPTS"; fi

# include helpful functions.
source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

# LIBDIR is an older variable that points at the root of the yeti code.
export LIBDIR=$FEISTY_MEOW_DIR

if [ -z "$FEISTY_MEOW_GENERATED" ]; then
  # The generated scripts directory is where automatically generated files live.
  # It is separate from the main body of the shell scripts in order to keep things from
  # exploding.
  export FEISTY_MEOW_GENERATED=$HOME/.zz_auto_gen
fi

##############

# user variables...

# define a default name, if one wasn't already set.
if [ -z "$NAME" ]; then
  export NAME='Unset Q. Namington, Fixley Your Name III'
fi

##############


##############################################################################
# other variables...
##############################################################################

# pull in the custom overrides for feisty_meow scripts.
for i in $FEISTY_MEOW_GENERATED/custom/*.sh; do
  if [ ! -f "$i" ]; then
    # skip it if it's not real.
    continue;
  fi
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "loading customization: $(basename $(dirname $i))/$(basename $i)"
  fi
  source $i
done

# sets the prompts to what we (i.e., i) like...
# there are four different prompts.  the first one, PS1, is the one that users
# see the most often. 
export PS1='\u@\h $ ';
### export PS2='> '; export PS3='#? '; export PS4='+ '

# variables for perl.
export PERLLIB
if [ "$OS" != "Windows_NT" ]; then
  PERLLIB+="/usr/lib/perl5"
else
  export PERLIO=:perlio
    # choose perl's IO over the ms-windows version so we can handle file
    # bytes properly.
fi

# iterate across our sub-directories and find the perl scripts.
# this currently only looks one level down.
for i in $FEISTY_MEOW_SCRIPTS/*; do
  if [ -d "$i" ]; then
    # check if there is a perl file present; add the folder to PERLLIB if so.
    ls $i/*.pl &>/dev/null
    if [ $? -eq 0 ]; then
      PERLLIB+=":$i"
    fi
  fi
done
#echo PERLLIB is now $PERLLIB

# set this so nechung can find its data.
export NECHUNG=$LIBDIR/database/fortunes.dat

# ensure we use the right kind of rsh for security.
export CVS_RSH=ssh

# sets the history length and max file size so we can get some long history around here.
HISTSIZE=1000000
HISTFILESIZE=2000000

# set the editor for subversion if it hasn't already been set.
if [ -z "$SVN_EDITOR" ]; then
#hmmm: not sure what original reason for having these different was...
  if [ "$OS"  == "Windows_NT" ]; then
    export SVN_EDITOR=$(which gvim)
  else
    export SVN_EDITOR=$(which vi)
  fi
fi

# include variables needed for compiling hoople and using its scripts.
if [ -z "$FEISTY_MEOW_DIR" ]; then
  if [ -d "$HOME/feisty_meow" ]; then
    export FEISTY_MEOW_DIR="$HOME/feisty_meow"
  fi
fi

# initialize the build variables, if possible.
found_build_vars=0
if [ ! -z "$FEISTY_MEOW_DIR" ]; then
  # first guess at using the old school bin directory.
  bv="$FEISTY_MEOW_DIR/bin/build_variables.sh"
  if [ -f "$bv" ]; then
    # the old bin directory is present, so let's use its build vars.
    source "$bv" "$bv"
    found_build_vars=1
  else
    # try again with the new school location for the file.
    bv="$FEISTY_MEOW_DIR/scripts/generator/build_variables.sh"
    if [ -f "$bv" ]; then
      # yep, that one looks good, so pull in the build defs.
      source "$bv" "$bv"
      found_build_vars=1
    else
      # try once more with new school and assume we're deep.
      bv="$FEISTY_MEOW_DIR/../../scripts/generator/build_variables.sh"
      if [ -f "$bv" ]; then
        # sweet, there is something there.
        source "$bv" "$bv"
        found_build_vars=1
      fi
    fi
  fi
fi

# augment the configuration if we found our build variables.
if [ $found_build_vars == 1 ]; then

  # the binary directory contains handy programs we use a lot in yeti.  we set up the path to it
  # here based on the operating system.
  # note that yeti has recently become more dependent on hoople.  hoople was always the source of
  # the binaries, but now we don't ship them with yeti any more as pre-built items.  this reduces
  # the size of the code package a lot and shortens up our possible exposure to compromised
  # binaries.  people can bootstrap up their own set from hoople now instead.
  export BINDIR=$FEISTY_MEOW_DIR/production/binaries

  # add binaries created within build to the path.
  export PATH="$(dos_to_msys_path $BUILD_TOP/build/bin):$PATH"

  # Shared libraries are located via this variable.
  export LD_LIBRARY_PATH="$(dos_to_msys_path $LD_LIBRARY_PATH):$(dos_to_msys_path $BINDIR)"
fi

# Set the path for locating applications.
export PATH="$(dos_to_msys_path $BINDIR):$(dos_to_msys_path $FEISTY_MEOW_GENERATED):$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/lib:/usr/games:/usr/bin:."

if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization ends....; fi

