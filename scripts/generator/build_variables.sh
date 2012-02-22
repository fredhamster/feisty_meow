##############
#
#  Name   : build variable calculator
#  Author : Chris Koeritz
#
#  Purpose:
#
#    This script sets up all the variables needed by the HOOPLE system for
#  building the source code.  It can either be run as a bash script directly
#  like so:
#
#      bash ~/feisty_meow/scripts/generator/build_variables.sh
#
#  which will establish a new shell containing all the variables, or you can
#  'source' the script like so:
#
#      build_vars=~/feisty_meow/scripts/generator/build_variables.sh
#      source $build_vars $build_vars
#
#  to set all of the variables in your current shell.  The full path is
#  necessary in these commands to allow the script to easily find itself.
#  The 'source' version needs to be fed the actual path to the script
#  because bash 'source' commands cause the first parameter (called $0) to
#  be set to just the path to bash itself.
#
##############
# Copyright (c) 2004-$now By Author.  This program is free software; you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either version 2 of
# the License or (at your option) any later version.  This is online at:
#     http://www.fsf.org/copyleft/gpl.html
# Please send any updates to: fred@gruntose.com
##############

# here is where we compute the locations for the build's pieces, based on
# where this script is located.  we currently assume that the build scripts
# like this one are at the same height in the hierarchy as the clam scripts
# that are used in the bootstrapping process.

# get the most important bits first; the directory this script lives in and
# the script's name.
PARM_0="$0"
PARM_1="$1"

##############

# helpful build function zone.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

##############

# perform some calculations to get the right paths from our parameters.
if [ ! -z "$PARM_1" ]; then
  # use the first real parameter since this is probably the 'source' version.
  export BUILD_SCRIPTS_DIR="$(dirname "$PARM_1")"
  THIS_TOOL_NAME="$(basename "$PARM_1")"
echo ==sourced version buildscriptsdir is $BUILD_SCRIPTS_DIR
else
  # use the zeroth parameter, since we know nothing more about our name.
  export BUILD_SCRIPTS_DIR="$(dirname "$PARM_0")"
  THIS_TOOL_NAME="$(basename "$PARM_0")"
echo ==bashed version buildscriptsdir is $BUILD_SCRIPTS_DIR
fi
BUILD_SCRIPTS_DIR="$(cd $(echo $BUILD_SCRIPTS_DIR | tr '\\\\' '/' ); \pwd)"
echo "==buildvars buildscriptsdir is $BUILD_SCRIPTS_DIR"

# figure out the other paths based on where we found this script.
export BUILDING_HIERARCHY="$(echo "$BUILD_SCRIPTS_DIR" | sed -e 's/\(.*\)\/[^\/]*/\1/')"
export CLAM_DIR="$(cd $BUILD_SCRIPTS_DIR/../clam ; \pwd)"
echo "==buildvars clamdir is $CLAM_DIR"
# synonym to make other builds happy.
export BUILDER_DIR="$BUILDING_HIERARCHY"

# guess the current platform.
IS_UNIX=$(uname | grep -i linux)
if [ -z "$IS_UNIX" ]; then IS_UNIX=$(uname | grep -i unix); fi
if [ -z "$IS_UNIX" ]; then IS_UNIX=$(uname | grep -i darwin); fi
IS_DOS=$(uname | grep -i ming)
if [ -z "$IS_DOS" ]; then IS_DOS=$(uname | grep -i cygwin); fi

# set some clam parameters for compilation.  if the script can't guess the
# right configuration, then you will need to set them in the last 'else'
# below.
if [ ! -z "$IS_UNIX" ]; then export OPERATING_SYSTEM=UNIX;
elif [ ! -z "$IS_DOS" ]; then export OPERATING_SYSTEM=WIN32;
else
  # the system is unknown, so we give up on guessing.
  export OPERATING_SYSTEM=unknown
fi
if [ ! -z "$SHELL_DEBUG" ]; then
  echo "[OS is \"$OPERATING_SYSTEM\"]"
fi

if [ ! -z "$SHELL_DEBUG" ]; then
  echo "[FEISTY_MEOW_DIR is $FEISTY_MEOW_DIR]"
fi

# new BUILD_TOP variable points at the utter top-most level of any files
# in the building hierarchy.
export BUILD_TOP="$FEISTY_MEOW_DIR"
echo build top is $BUILD_TOP

# this variable points at a folder where we store most of the generated products
# of the build.  these tend to be the things that will be used for packaging into
# different types of products.
export PRODUCTION_DIR="$BUILD_TOP/production"

# we define a log file storage area that can be relied on by the build.
export LOGS_DIR="$PRODUCTION_DIR/logs"
if [ ! -d "$LOGS_DIR" ]; then
  mkdir -p "$LOGS_DIR"
fi

# hook clam into the compilation system.
function make()
{
  /usr/bin/make -I "$CLAM_DIR" $*
}

##############

# debugging area where we say what we think we know.

#echo scripts: $BUILD_SCRIPTS_DIR
#echo build tools hier: $BUILDING_HIERARCHY
#echo this tool: $THIS_TOOL_NAME
#echo repository: $FEISTY_MEOW_DIR
#echo clam: $CLAM_DIR
#echo makeflags: $MAKEFLAGS

##############

# test out our computed variables to make sure they look right.
pushd / &>/dev/null # jump to the root so relative paths are caught.

# flag for whether any checks have failed.
got_bad=

# first the scripts directory; do we find this script there?
if [ ! -f "$BUILD_SCRIPTS_DIR/$THIS_TOOL_NAME" ]; then
  echo "This script cannot locate the proper build folders.  The crucial path"
  echo "variable seems to be '$BUILD_SCRIPTS_DIR', which"
  echo "does not seem to contain '$THIS_TOOL_NAME' (this"
  echo "script's apparent name)."
  got_bad=1
fi

# next the clam directory; is the main variables file present there?
if [ -z "$got_bad" -a ! -f "$CLAM_DIR/variables.def" ]; then
  echo "The clam directory could not be located under our build tools hierarchy."
  echo "Please examine the configuration and make sure that this script is in a"
  echo "directory that resides at the same height as the 'clam' directory."
  got_bad=1
fi

# now compute some more paths with a bit of "heuristics" for where we can
# find the source code.
export TOOL_SOURCES="$FEISTY_MEOW_DIR/nucleus/tools"
echo "==tool source is $TOOL_SOURCES"
if [ -z "$got_bad" -a ! -d "$TOOL_SOURCES/dependency_tool" -o ! -d "$TOOL_SOURCES/clam_tools" ]; then
  echo "This script cannot locate the tool source code folder.  This is where the"
  echo "dependency_tool and clam_tools folders are expected to be."
  got_bad=1
fi

############################
  
# we only run the rest of the script if we know we didn't have some kind of
# bad thing happen earlier.
if [ -z "$got_bad" ]; then

  # where we store the binaries used for building the rest of the code base.
  export BINARY_DIR="$PRODUCTION_DIR/clam_bin"
    # the final destination for the new binaries which provide the hoople
    # build with all the apps it needs to get going.
  export TARGETS_DIR="$PRODUCTION_DIR/binaries"
    # targets directory is meaningful to clam, which will use it for output.
  export INTERMEDIATE_EXE_DIR="$TARGETS_DIR"
    # where we are building the apps before they get promoted.

  export WASTE_DIR="$PRODUCTION_DIR/waste"
  if [ ! -d "$WASTE_DIR" ]; then
    mkdir -p "$WASTE_DIR"
  fi
  export TEMPORARIES_DIR="$WASTE_DIR/temporaries"
  if [ ! -d "$TEMPORARIES_DIR" ]; then
    mkdir -p "$TEMPORARIES_DIR"
  fi
  
  # calculate which build ini file to use.
  export BUILD_PARAMETER_FILE="$PRODUCTION_DIR/feisty_meow_config.ini"
  if [ ! -f "$BUILD_PARAMETER_FILE" ]; then
    echo "Cannot find a useful build configuration file."
  fi
  
  # pick the executable's file ending based on the platform.
  if [ "$OPERATING_SYSTEM" == "UNIX" ]; then export EXE_ENDING=;
  elif [ "$OPERATING_SYSTEM" == "WIN32" ]; then export EXE_ENDING=.exe;
  else
    echo "The OPERATING_SYSTEM variable is unset or unknown.  Bailing out."
  fi
  
  # we should have established our internal variables now, so let's try
  # using them.
  export PATH=$BINARY_DIR:$PATH
  
  # load up the helper variables for visual studio on winders.
  if [ "$OPERATING_SYSTEM" == "WIN32" ]; then
    source "$BUILD_SCRIPTS_DIR/vis_stu_vars.sh"
  else
    export LD_LIBRARY_PATH="$TARGETS_DIR"
  fi
  
  popd &>/dev/null # checking is over, jump back to the starting point.
  
  ############################
  
  # at this point, all the build related variables should be valid.
  
  if [ -z "$INCLUDED_FROM_BOOTSTRAP" \
      -a -z "$PARM_1" ]; then
    # we are running as a stand-alone script, so we stay resident with our
    # current set of variables.
    bash
  fi

fi

