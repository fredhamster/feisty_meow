###############################################################################
#                                                                             #
#  Name   : build variable calculator                                         #
#  Author : Chris Koeritz                                                     #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    This script sets up all the variables needed by the HOOPLE system for    #
#  building the source code.  It can either be run as a bash script directly  #
#  like so:                                                                   #
#                                                                             #
#      bash ~/feisty_meow/scripts/generator/build_variables.sh                #
#                                                                             #
#  which will establish a new shell containing all the variables, or you can  #
#  'source' the script like so:                                               #
#                                                                             #
#      build_vars=~/feisty_meow/scripts/generator/build_variables.sh          #
#      source $build_vars $build_vars                                         #
#                                                                             #
#  to set all of the variables in your current shell.  The full path is       #
#  necessary in these commands to allow the script to easily find itself.     #
#  The 'source' version needs to be fed the actual path to the script         #
#  because bash 'source' commands cause the first parameter (called $0) to    #
#  be set to just the path to bash itself.                                    #
#                                                                             #
###############################################################################
# Copyright (c) 2004-$now By Author.  This program is free software; you can  #
# redistribute it and/or modify it under the terms of the GNU General Public  #
# License as published by the Free Software Foundation; either version 2 of   #
# the License or (at your option) any later version.  This is online at:      #
#     http://www.fsf.org/copyleft/gpl.html                                    #
# Please send any updates to: fred@gruntose.com                               #
###############################################################################

# prerequisites for this script:
#
# (1) the script should be run with a full path, so that it can decide where
#     it lives with minimal fuss.
# (2) on windows, the msys bin directory should already be in the path so that
#     tools like dirname are already available.

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

# switches from a /X/path form to an X:/ form.
function msys_to_dos_path() {
  # we always remove dos slashes in favor of forward slashes.
  echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/'
}

# switches from an X:/ form to an /X/path form.
function dos_to_msys_path() {
  # we always remove dos slashes in favor of forward slashes.
  echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/\1\/\2/'
}

###hmmm: move test suite out to the functions file in yeti, where the definitive versions
#        of dos to msys etc live.
# test suite for above functions.
#echo this should go from msys to dos:
#  prior='/c/bogart\dingle'
#  latter=$(msys_to_dos_path "$prior")
#  echo went from $prior to $latter
#echo this should go from dos to msys:
#  prior='D:\bogart\dingle'
#  latter=$(dos_to_msys_path "$prior")
#  echo went from $prior to $latter

##############

# perform some calculations to get the right paths from our parameters.
if [ ! -z "$PARM_1" ]; then
  # use the first real parameter since this is probably the 'source' version.
  export BUILD_SCRIPTS_DIR="$(dirname "$PARM_1")"
  THIS_TOOL_NAME="$(basename "$PARM_1")"
#echo sourced version buildscriptsdir is $BUILD_SCRIPTS_DIR
else
  # use the zeroth parameter, since we know nothing more about our name.
  export BUILD_SCRIPTS_DIR="$(dirname "$PARM_0")"
  THIS_TOOL_NAME="$(basename "$PARM_0")"
#echo bashed version buildscriptsdir is $BUILD_SCRIPTS_DIR
fi
BUILD_SCRIPTS_DIR="$(echo $BUILD_SCRIPTS_DIR | tr '\\\\' '/' )"
#echo post tr buildscriptsdir is $BUILD_SCRIPTS_DIR

# figure out the other paths based on where we found this script.
export BUILDING_HIERARCHY="$(echo "$BUILD_SCRIPTS_DIR" | sed -e 's/\(.*\)\/[^\/]*/\1/')"
export CLAM_DIR="$BUILD_SCRIPTS_DIR/../clam"
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

# we create the variable FEISTY_MEOW_DIR, but we keep the dos form of
# the path, because otherwise lots of bad things happens when passing the
# folders around to visual studio commands that don't allow a space after them.
if [ -d "$BUILDING_HIERARCHY/source" ]; then
  # old style repository is same height as building hierarchy.
  export FEISTY_MEOW_DIR="$BUILDING_HIERARCHY"
else
  # new style repository is a level above the build hierarchy.
  export FEISTY_MEOW_DIR="$(echo "$BUILDING_HIERARCHY" | sed -e 's/\(.*\)\/[^\/]*/\1/')"
fi

if [ "$OPERATING_SYSTEM" = "WIN32" ]; then
  # make sure repository dir looks right on windoze.
  export FEISTY_MEOW_DIR="$(msys_to_dos_path "$FEISTY_MEOW_DIR")"
fi

if [ ! -z "$SHELL_DEBUG" ]; then
  echo "[FEISTY_MEOW_DIR is $FEISTY_MEOW_DIR]"
fi

# new BUILD_TOP variable points at the utter top-most level of any files
# in the building hierarchy.
export BUILD_TOP="$FEISTY_MEOW_DIR"

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

# first the scripts directory; do we find this script there?
if [ ! -f "$BUILD_SCRIPTS_DIR/$THIS_TOOL_NAME" ]; then
  echo "This script cannot locate the proper build folders.  The crucial path"
  echo "variable seems to be '$BUILD_SCRIPTS_DIR', which"
  echo "does not seem to contain '$THIS_TOOL_NAME' (this"
  echo "script's apparent name)."
fi

# next the clam directory; is the main variables file present there?
if [ ! -f "$CLAM_DIR/variables.def" ]; then
  echo "The clam directory could not be located under our build tools hierarchy."
  echo "Please examine the configuration and make sure that this script is in a"
  echo "directory that resides at the same height as the 'clam' directory."
fi

# now compute some more paths with a bit of "heuristics" for where we can
# find the source code.
export TOOL_SOURCES="$FEISTY_MEOW_DIR/nucleus/tools"
if [ ! -d "$TOOL_SOURCES/dependency_tool" -o ! -d "$TOOL_SOURCES/clam_tools" ]; then
  if [ ! -d "$TOOL_SOURCES/dependency_tool" -o ! -d "$TOOL_SOURCES/clam_tools" ]; then
    echo "This script cannot locate the tool source code folder.  This is where the"
    echo "dependency_tool and clam_tools folders are expected to be."
  fi
fi

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

############################################################################

# at this point, all the build related variables should be valid.

if [ -z "$INCLUDED_FROM_BOOTSTRAP" \
    -a -z "$PARM_1" ]; then
  # we are running as a stand-alone script, so we stay resident with our
  # current set of variables.
  bash
fi

############################################################################

