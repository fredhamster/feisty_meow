#!/bin/bash

##############

# variables script:
#   Defines the environment variables used by the personalized unix
#   environment.
# Author: Chris Koeritz

##############

# we'll run this again only if we think it's needed.
if [ -z "$NECHUNG" ]; then

  if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization begins...; fi
  
  ##############
  
  # start with some simpler things.
  
  export SCRIPT_SYSTEM=feisty_meow
  
  # OS variable records the operating system we think we found.
  if [ -z "$OS" ]; then
    export OS=UNIX
  fi
  export IS_DARWIN=$(echo $OSTYPE | grep -i darwin)
  
  ##############
  
  # fallbacks to set crucial variables for feisty meow...
  
  # set the main root directory variable for the feisty meow codebase.
  # this is only used for extreme failure modes, when the values were not
  # pulled in from our auto-generated config.
  if [ -z "$FEISTY_MEOW_DIR" ]; then
    if [ -d "$HOME/feisty_meow" ]; then
      export FEISTY_MEOW_DIR="$HOME/feisty_meow"
      export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_DIR/scripts"
    fi
  fi
  
  # similarly, make sure we have someplace to look for our generated files, if
  # we were not handed a value.
  if [ -z "$FEISTY_MEOW_GENERATED" ]; then
    # The generated scripts directory is where automatically generated files live.
    # It is separate from the main body of the shell scripts in order to keep things from
    # exploding.
    export FEISTY_MEOW_GENERATED=$HOME/.zz_auto_gen
  fi
  
  ##############
  
  # umask sets a permission mask for all file creations.  the mask used here
  # disallows writing by the "group" and "others" categories.
  umask 022
  # ulimit sets user limits.  we set the maximum allowed core dump file size
  # to zero, because it is obnoxious to see the core dumps from crashed
  # programs lying around everywhere.
  ulimit -c 0
  
  ##############
  
  # include helpful functions.
  source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
  
  ##############
  
  # user variables, sort of...  if they haven't given themselves a name yet,
  # then we will make one up for them.
  
  # define a default name, if one wasn't already set.
  if [ -z "$NAME" ]; then
    export NAME='Unset Q. Namington, Fixley Your Name III'
  fi
  
  ##############
  
  # sets the main prompt to a simple default, with user@host.
  export PS1='\u@\h $ ';
  
  ##############
  
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
  
  ##############
  
  # set this so nechung can find its data.
  export NECHUNG=$FEISTY_MEOW_DIR/database/fortunes.dat
  
  # ensure we use the right kind of rsh for security.
  export CVS_RSH=ssh
  
  # sets the history length and max file size so we can get some long history around here.
  HISTSIZE=1000000
  HISTFILESIZE=2000000
  
  # the base checkout list is just to update feisty_meow.  additional folder
  # names can be added in your customized scripts.
  export REPOSITORY_LIST="feisty_meow"
  
  # set the editor for subversion if it hasn't already been set.
  if [ -z "$SVN_EDITOR" ]; then
  #hmmm: not sure what original reason for having these different was...
    if [ "$OS"  == "Windows_NT" ]; then
      export SVN_EDITOR=$(which gvim)
    else
      export SVN_EDITOR=$(which vi)
    fi
  fi
  
  # initializes the feisty meow build variables, if possible.
  function initialize_build_variables()
  {
    found_build_vars=0
    # we need to know the feisty meow directory, or we bail.
    if [ -z "$FEISTY_MEOW_DIR" ]; then return; fi
    # pick from our expected generator folder, but make sure it's there...
    buildvars="$FEISTY_MEOW_DIR/scripts/generator/build_variables.sh"
    if [ -f "$buildvars" ]; then
      # yep, that one looks good, so pull in the build defs.
      source "$buildvars" "$buildvars"
      found_build_vars=1
    fi
    # now augment the environment if we found our build variables.
    if [ $found_build_vars == 1 ]; then
      # the binary directory contains handy programs we use a lot in yeti.  we set up the path to it
      # here based on the operating system.
      # note that yeti has recently become more dependent on hoople.  hoople was always the source of
      # the binaries, but now we don't ship them with yeti any more as pre-built items.  this reduces
      # the size of the code package a lot and shortens up our possible exposure to compromised
      # binaries.  people can bootstrap up their own set from hoople now instead.
      export BINDIR=$FEISTY_MEOW_DIR/production/binaries
  
      # add binaries created within build to the path.
#    export PATH="$(dos_to_msys_path $BUILD_TOP/build/bin):$PATH"
      export PATH="$BINDIR:$PATH"
  
      # Shared libraries are located via this variable.
#    export LD_LIBRARY_PATH="$(dos_to_msys_path $LD_LIBRARY_PATH):$(dos_to_msys_path $BINDIR)"
      export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$BINDIR"
    fi
  }
  
  # load in the build environment.
  initialize_build_variables
  
  ##############
  
  # windoze specific patching up missing things.
  
  if [ "$OS" == "Windows_NT" ]; then
    export HOSTNAME=$(echo $HOSTNAME | tr A-Z a-z)
  fi
  
  ##############
  
  # pull in the custom overrides for feisty_meow scripts.  this is done last,
  # because we want to set everything up as expected, then let the user
  # override individual variables and definitions.
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
  
  ##############
  
  # set the path for locating applications.  this is done after any
  # potential overrides from the user.
  #export PATH="$(dos_to_msys_path $BINDIR):$(dos_to_msys_path $FEISTY_MEOW_GENERATED):$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/lib:/usr/games:/usr/bin:."
  export PATH="$FEISTY_MEOW_GENERATED:$PATH:/sbin:."
###noise! :/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/lib:/usr/games:/usr/bin:.
  
  ##############
  
  if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization ends....; fi

else
  # we are seeing this lot again, we believe, so make things easier for the
  # rest of initialization.
  export LIGHTWEIGHT_INIT=true
  if [ ! -z "$SHELL_DEBUG" ]; then
 echo skipped var init and put in lightweight mode due to belief in previous existence.; 
fi
fi

