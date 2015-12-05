#!/bin/bash

##############

# variables script:
#   Defines the environment variables used by the personalized unix
#   environment.
# Author: Chris Koeritz

##############

#hmmm: moved from functions.sh; does that hose everything up?

  # defines a variable within the feisty meow environment and remembers that
  # this is a new or modified definition.  if the feisty meow codebase is
  # unloaded, then so are all the variables that were defined.
  # this function always exports the variables it defines.
  function define_yeti_variable()
  {
# if variable exists already, save old value for restore,
# otherwise save null value for restore,
# have to handle unsetting if there was no prior value of one
# we newly defined.
# add variable name to a list of feisty defined variables.

#hmmm: first implem just sets it up and exports the variable.
#  i.e., this method always exports.
export "${@}" 


return 0
  }


##############

# this section should always run or bash will reset them on us.
# these need to be as minimal as possible.

# sets the main prompt to a simple default, with user@host.
define_yeti_variable PS1='\u@\h $ ';
# sets the history length and max file size so we can get some long history around here.
define_yeti_variable HISTSIZE=1000000
define_yeti_variable HISTFILESIZE=8000000

# make the TERM available to all sub-shells.
define_yeti_variable TERM
  
##############
  
# we'll run this again only if we think it's needed.
if [ -z "$NECHUNG" ]; then

  if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization begins...; fi
  
  ##############
  
  # start with some simpler things.
  
  define_yeti_variable SCRIPT_SYSTEM=feisty_meow
  
  # OS variable records the operating system we think we found.
  if [ -z "$OS" ]; then
    define_yeti_variable OS=UNIX
  fi
  define_yeti_variable IS_DARWIN=$(echo $OSTYPE | grep -i darwin)
  
  ##############

  # guess the current platform.
  IS_UNIX=$(uname | grep -i linux)
  if [ -z "$IS_UNIX" ]; then IS_UNIX=$(uname | grep -i unix); fi
  if [ -z "$IS_UNIX" ]; then IS_UNIX=$(uname | grep -i darwin); fi
  IS_DOS=$(uname | grep -i ming)
  if [ -z "$IS_DOS" ]; then IS_DOS=$(uname | grep -i cygwin); fi

  # now if we're stuck in DOS, try to determine the type of system.
  if [ ! -z "$IS_DOS" ]; then
    # IS_MSYS will be non-empty if this is the msys toolset.  otherwise
    # we assume that it's cygwin.
    IS_MSYS=$(uname | grep -i ming)
  fi

  ##############
  
  # fallbacks to set crucial variables for feisty meow...
  
  # set the main root directory variable for the feisty meow codebase.
  # this is only used for extreme failure modes, when the values were not
  # pulled in from our auto-generated config.
  if [ -z "$FEISTY_MEOW_APEX" ]; then
    if [ -d "$HOME/feisty_meow" ]; then
      define_yeti_variable FEISTY_MEOW_APEX="$HOME/feisty_meow"
      define_yeti_variable FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_SCRIPTS"
    fi
  fi

  # main declaration of the transients area.
  if [ -z "$TMP" ]; then
    define_yeti_variable TMP=$HOME/.tmp
  fi

  # set up the top-level for all build creations and logs and such.
  if [ -z "$GENERATED_DIR" ]; then
    define_yeti_variable GENERATED_DIR="$TMP/generated-feisty_meow"
  fi
  if [ ! -d "$GENERATED_DIR" ]; then
    mkdir -p "$GENERATED_DIR"
  fi
  # set up our effluent outsourcing valves.
  if [ -z "$TEMPORARIES_DIR" ]; then
    define_yeti_variable TEMPORARIES_DIR="$GENERATED_DIR/temporaries"
  fi
  if [ ! -d "$TEMPORARIES_DIR" ]; then
    mkdir -p "$TEMPORARIES_DIR"
  fi

  # similarly, make sure we have someplace to look for our generated files, if
  # we were not handed a value.
  if [ -z "$FEISTY_MEOW_LOADING_DOCK" ]; then
    # The generated scripts directory is where automatically generated files live.
    # It is separate from the main body of the shell scripts in order to keep things from
    # exploding.
    define_yeti_variable FEISTY_MEOW_LOADING_DOCK=$HOME/.zz_feisty_loading
  fi
  
  ##############
  
  # umask sets a permission mask for all file creations.
  # this mask disallows writes by "group" and "others".
  umask 022
  # this mask disallows writes by the "group" and disallows "others" completely.
  #umask 027

  # ulimit sets user limits.  we set the maximum allowed core dump file size
  # to zero, because it is obnoxious to see the core dumps from crashed
  # programs lying around everywhere.
  ulimit -c 0
  
  ##############
  
  # user variables, sort of...  if they haven't given themselves a name yet,
  # then we will make one up for them.
  
  # define a default name, if one wasn't already set.
  if [ -z "$NAME" ]; then
    define_yeti_variable NAME='Unset Q. Namington, Fixley Your Name III'
  fi
  
  ##############
  
  # variables for perl.
  
  define_yeti_variable PERLLIB+="/usr/lib/perl5"
  if [ "$OS" == "Windows_NT" ]; then
    define_yeti_variable PERLIO=:perlio
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
        PERLLIB+=":$(dos_to_unix_path $i)"
      fi
    fi
  done
  #echo PERLLIB is now $PERLLIB
  
  ##############
  
  # set this so nechung can find its data.
  define_yeti_variable NECHUNG=$FEISTY_MEOW_APEX/infobase/fortunes.dat
  
  # ensure we use the right kind of secure shell.
#  define_yeti_variable CVS_RSH=$FEISTY_MEOW_SCRIPTS/security/ssh.sh
#  define_yeti_variable GIT_SSH=$FEISTY_MEOW_SCRIPTS/security/ssh.sh
  
  # the base checkout list is just to update feisty_meow.  additional folder
  # names can be added in your customized scripts.  the space at the end of
  # this variable is important and allows users to extend the list like:
  #    define_yeti_variable REPOSITORY_DIR+="muppets configs"
  define_yeti_variable REPOSITORY_LIST="feisty_meow "
  
  # initializes the feisty meow build variables, if possible.
  function initialize_build_variables()
  {
    found_build_vars=0
    # we need to know the feisty meow directory, or we bail.
    if [ -z "$FEISTY_MEOW_APEX" ]; then return; fi
    # pick from our expected generator folder, but make sure it's there...
    buildvars="$FEISTY_MEOW_SCRIPTS/generator/build_variables.sh"
    if [ -f "$buildvars" ]; then
      # yep, that one looks good, so pull in the build defs.
      source "$buildvars" "$buildvars"
      found_build_vars=1
    fi
    # now augment the environment if we found our build variables.
    if [ $found_build_vars == 1 ]; then
      # the binary directory contains our collection of handy programs.
      define_yeti_variable BINDIR=$TARGETS_DIR
      # add binaries created within build to the path.
      define_yeti_variable PATH="$(dos_to_unix_path $BINDIR):$PATH"
      # Shared libraries are located via this variable.
      define_yeti_variable LD_LIBRARY_PATH="$(dos_to_unix_path $LD_LIBRARY_PATH):$(dos_to_unix_path $BINDIR)"
    fi
  }
  
  ##############
  
  # windoze specific patching up missing things.
  
  if [ "$OS" == "Windows_NT" ]; then
    define_yeti_variable HOSTNAME=$(echo $HOSTNAME | tr A-Z a-z)
  fi
  
  ##############
  
  # load in the build environment.
  initialize_build_variables
  
  ##############
  
  # add to the PATH variables used for locating applications.  this step is taken after any
  # potential overrides from the user.
  define_yeti_variable PATH="$(dos_to_unix_path $FEISTY_MEOW_LOADING_DOCK):$PATH:$(find /usr/local/games -maxdepth 1 -type d -exec echo -n {}: ';' 2>/dev/null)/sbin"
  
  ##############

  # set the SHUNIT_DIR so our shunit tests can find the codebase.
  define_yeti_variable SHUNIT_DIR="$FEISTY_MEOW_SCRIPTS/shunit"
  
  ##############
  
  if [ ! -z "$SHELL_DEBUG" ]; then echo variables initialization ends....; fi
fi

##############

# pull in the custom overrides for feisty_meow scripts.  this is done last,
# because we want to set everything up as expected, then let the user
# override individual variables and definitions.  we also don't guard this
# to avoid running it again, because we don't know what mix of functions and
# aliases they want to define in there.
for i in $FEISTY_MEOW_LOADING_DOCK/custom/*.sh; do
  if [ ! -f "$i" ]; then
    # skip it if it's not real.
    continue;
  fi
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "loading customization: $(basename $(dirname $i))/$(basename $i)"
  fi
  source "$i"
done
  
