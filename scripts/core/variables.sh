#!/bin/bash

##############################################################################
# variables script:
#   Defines the environment variables used by the personalized unix
#   environment.
##############################################################################

if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization begins...; fi

##############################################################################
# System variables.
##############################################################################
# OS stands for the operating system that we think is running.
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

##############################################################################
# Directory variables.
##############################################################################
# The yeti library directory holds useful shell scripts, public databases,
# configuration examples, javascript code, and other stuff.
export SCRIPT_SYSTEM=feisty_meow

#if [ -z "$YETI_DIR" ]; then export YETI_DIR="$HOME/$SCRIPT_SYSTEM"; fi
#if [ -z "$YETI_SCRIPTS" ]; then export YETI_SCRIPTS="$YETI_DIR/scripts"; fi
#if [ -z "$SHELLDIR" ]; then export SHELLDIR="$YETI_SCRIPTS"; fi

# include helpful functions.
source "$YETI_SCRIPTS/core/functions.sh"

# LIBDIR is an older variable that points at the root of the yeti code.
export LIBDIR=$YETI_DIR

if [ -z "$GENERADIR" ]; then
  # The generated scripts directory is where automatically generated files live.
  # It is separate from the main body of the shell scripts in order to keep things from
  # exploding.
  export GENERADIR=$HOME/.zz_auto_gen
fi

##############################################################################
# other variables...
##############################################################################

# pull in the custom variable overrides for bash.
if [ -f "$YETI_SCRIPTS/custom/c_variables.sh" ]; then
  source "$YETI_SCRIPTS/custom/c_variables.sh"
fi

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
#echo "the scripts dir is $YETI_SCRIPTS"
  YETI_SCRIPTS="$(echo $YETI_SCRIPTS | sed -e 's/\\/\//g')"
  SHELLDIR="$YETI_SCRIPTS"
#echo "the scripts dir is now $SHELLDIR"
  export PERLIO=:perlio
    # choose perl's IO over the system's so we can handle file bytes exactly.
fi

#make this automatic!
PERLLIB+=":$YETI_SCRIPTS/core:$YETI_SCRIPTS/text:$YETI_SCRIPTS/files:$YETI_SCRIPTS/archival"

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
if [ -z "$REPOSITORY_DIR" ]; then
  if [ -d "$HOME/feisty_meow" ]; then
    export REPOSITORY_DIR="$HOME/feisty_meow"
  fi
fi

# initialize the build variables, if possible.
found_build_vars=0
if [ ! -z "$REPOSITORY_DIR" ]; then
  # first guess at using the old school bin directory.
  bv="$REPOSITORY_DIR/bin/build_variables.sh"
  if [ -f "$bv" ]; then
    # the old bin directory is present, so let's use its build vars.
    source "$bv" "$bv"
    found_build_vars=1
  else
    # try again with the new school location for the file.
    bv="$REPOSITORY_DIR/scripts/generator/build_variables.sh"
    if [ -f "$bv" ]; then
      # yep, that one looks good, so pull in the build defs.
      source "$bv" "$bv"
      found_build_vars=1
    else
      # try once more with new school and assume we're deep.
      bv="$REPOSITORY_DIR/../../scripts/generator/build_variables.sh"
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
  export BINDIR=$REPOSITORY_DIR/production/binaries

  # add binaries created within build to the path.
  export PATH="$(dos_to_msys_path $BUILD_TOP/build/bin):$PATH"

  # Shared libraries are located via this variable.
  export LD_LIBRARY_PATH="$(dos_to_msys_path $LD_LIBRARY_PATH):$(dos_to_msys_path $BINDIR)"
fi

# Set the path for locating applications.
export PATH="$(dos_to_msys_path $BINDIR):$(dos_to_msys_path $GENERADIR):$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/lib:/usr/games:/usr/bin:."

if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization ends....; fi

