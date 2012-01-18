##############
#  Name   : initial setup script for HOOPLE
#  Author : Chris Koeritz
#  Purpose:
#    This script can bootstrap the HOOPLE libraries from a state where none   #
#  of the required binaries are built yet.  It will build the tools that the  #
#  CLAM system and HOOPLE need to get a build done.  Then the script builds   #
#  the whole source tree as a test of the code's overall health.              #
##############
# Copyright (c) 2004-$now By Author.  This program is free software; you can  #
# redistribute it and/or modify it under the terms of the GNU General Public  #
# License as published by the Free Software Foundation; either version 2 of   #
# the License or (at your option) any later version.  This is online at:      #
#     http://www.fsf.org/copyleft/gpl.html                                    #
# Please send any updates to: fred@gruntose.com                               #
##############

# prerequisites for this script:
#
# (1) the script should be run with a full path, so that it can decide where
#     it lives with minimal fuss.
# (2) on windows, the msys bin directory should already be in the path so that
#     tools like dirname are already available.

# make sure we know how to find our bash bins.
export PATH=/bin:$PATH
# signals that we're doing a fresh build to the variables script.
export INCLUDED_FROM_BOOTSTRAP=true
# pull in our build variables using the path to this script.
export BUILD_SCRIPTS_DIR="$( \cd "$(\dirname "$0")" && \pwd )"
echo build script initial from bootstrap: $BUILD_SCRIPTS_DIR
BUILD_SCRIPTS_DIR="$(echo $BUILD_SCRIPTS_DIR | tr '\\\\' '/' )"
echo build script after: $BUILD_SCRIPTS_DIR
# load in feisty meow basic scripts, if not already loaded.
source "$BUILD_SCRIPTS_DIR/../core/profile.sh"
# drop any previous version of the repository variable.
unset REPOSITORY_DIR
source "$BUILD_SCRIPTS_DIR/build_variables.sh" "$BUILD_SCRIPTS_DIR/build_variables.sh"

##############

# creates the directory for our binaries and gives it a reasonable paths configuration.
function prepare_binaries_dir()
{
  # we'll store binaries here from the bootstrap process.
  if [ ! -d "$BINARY_DIR" ]; then
    echo "creating binary dir now in $BINARY_DIR"
    mkdir "$BINARY_DIR"
  fi
  if [ ! -f "$BINARY_DIR/paths.ini" ]; then
    echo "copied paths.ini to binary dir."
    cp "$PRODUCTION_DIR/paths.ini" "$BINARY_DIR"
  fi
}

##############

# turn off sounds to avoid running the sound player that's not been built yet.
unset CLAM_ERROR_SOUND
unset CLAM_FINISH_SOUND

##############

echo "Build bootstrap process has started."

# preconditions for the build process...

# set up our output directories etc.
prepare_binaries_dir

# set a flag for this process so we can omit certain compilations as necessary.
export BOOT_STRAPPING=true

# enable this macro to get a much noisier build.
#export BE_NOISY=NOISY=t

##############

# these default flags turn off unnecessary support when we're rebuilding the
# minimal toolset needed for a successful build of hoople.
declare -a BUILD_DEFAULTS=( "BOOT_STRAPPING=t" "OPTIMIZE=t" "REBUILD=t" "DEBUG=" )
  # bootstrapping is always turned on for this particular script.
  # we also always optimize these builds and turn off the debug flag.
  # rebuild ensures that the new apps are made fresh: "REBUILD=t"
  #   it can be turned off when the build bootstrapper is being tested.
  # noisy can be added to spew lots of text: "NOISY=t"
  #   this can help with compilation issues by showing all the flags.

function make_code {
  make $* $BE_NOISY ${BUILD_DEFAULTS[@]}
  if [ $? != 0 ]; then
    echo "Failed to make on: $*"
    exit 2323
  fi
}

# removes pcdos eol from any scripts.  that assumes that the bootstrap script
# itself isn't polluted with them.
function strip_cr {
  ctrl_m=$'\015'
  for i in $*; do
    tempgrep="$(mktemp "$TEMPORARIES_DIR/tempgrep.XXXXXX")"
    grep -l "$ctrl_m" "$i" >$tempgrep
    if [ ! -z "$(cat $tempgrep)" ]; then
      temp="$(mktemp "$TEMPORARIES_DIR/tempsed.XXXXXX")"
      sed -e "s/$ctrl_m$//" <$i >$temp
      mv -f $temp $i
    fi
    rm "$tempgrep"
  done
}

# the promote function moves a file from the exe directory into the build's
# bin directory.  it performs the copy step and makes the file executable.
# the original name should just be the root of the filename without any
# extension.
# NOTE: this depends on the operating system having been chosen above!
if [ "$OPERATING_SYSTEM" = "UNIX" ]; then
  function promote {
    prepare_binaries_dir

    if [ ! -f "$INTERMEDIATE_EXE_DIR/$1" ]; then
      echo "Failed to build the application $1--quitting now."
      exit 1892
    fi
    cp "$INTERMEDIATE_EXE_DIR/$1" "$BINARY_DIR/$1"
    strip "$BINARY_DIR/$1"
    chmod 755 "$BINARY_DIR/$1"
  }
elif [ "$OPERATING_SYSTEM" = "WIN32" ]; then
  function promote {
    prepare_binaries_dir

    if [ ! -f "$INTERMEDIATE_EXE_DIR/$1.exe" ]; then
      echo "Failed to build the application $1.exe--quitting now."
      exit 1892
    fi
    cp "$INTERMEDIATE_EXE_DIR/$1.exe" "$BINARY_DIR"
    chmod 755 "$BINARY_DIR/$1.exe"
  }
else
  echo "The OPERATING_SYSTEM variable is unset or unknown.  Bailing out."
  exit 1822
fi

##############

# start the actual build process now...

# recreate our useful waste directories and other things...
source "$BUILD_SCRIPTS_DIR/build_variables.sh" "$BUILD_SCRIPTS_DIR/build_variables.sh"

# clean out any current contents.
bash "$BUILD_SCRIPTS_DIR/whack_build.sh" clean

# make this again so no one gets cranky.
mkdir -p "$LOGS_DIR"

echo "after removing and making dir again, it's here '$LOGS_DIR'..."
ls -al $LOGS_DIR

toolset_names=(makedep value_tagger version_stamper vsts_version_fixer write_build_config short_path sleep_ms zap_process playsound create_guid)

if [ -z "$SAVE_BINARIES" ]; then
  for i in ${toolset_names[*]}; do
    whack_name="$BINARY_DIR/$i$EXE_ENDING"
#echo removing "$whack_name"
    rm -f "$whack_name"
  done
fi

# make the clam shell scripts executable.
chmod 755 "$CLAM_DIR"/*.sh
chmod 755 "$CLAM_DIR"/cpp/*.sh
#chmod 755 "$CLAM_DIR"/csharp/*.sh

# rebuild the dependency tool.  needed by everything, pretty much, but
# since it's from the xfree project, it doesn't need any of our libraries.
if [ ! -f "$BINARY_DIR/makedep$EXE_ENDING" ]; then
  pushd "$TOOL_SOURCES/dependency_tool" &>/dev/null
  make_code pre_compilation NO_DEPS=t OMIT_VERSIONS=t
  make_code NO_DEPS=t OMIT_VERSIONS=t
  if [ ! -f "$INTERMEDIATE_EXE_DIR/makedep$EXE_ENDING" ]; then
    echo ""
    echo ""
    echo "The build of the makedep tool has failed.  Unknown causes...  Argh."
    echo ""
    exit 1820
  fi
  # make the tool available for the rest of the build.
  promote makedep
  popd &>/dev/null
fi

# rebuild the version tools and other support apps.
if [ ! -f "$BINARY_DIR/value_tagger$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/version_stamper$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/vsts_version_fixer$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/write_build_config$EXE_ENDING" ]; then
  pushd "$TOOL_SOURCES/clam_tools" &>/dev/null
  make_code pre_compilation OMIT_VERSIONS=t
  make_code OMIT_VERSIONS=t

#hmmm: really this should check all the expected apps.
#      nice to just have an array of the things built by this guy.
  if [ ! -f "$INTERMEDIATE_EXE_DIR/version_stamper$EXE_ENDING" ]; then
    echo ""
    echo ""
    echo "The build of the version_stamper tool has failed.  Unknown causes...  Argh."
    echo ""
    exit 1821
  fi

  promote value_tagger # tool scrambles through headers to standardize outcomes.
  promote version_stamper  # used for version stamping.
  promote vsts_version_fixer  # used for version stamping.
  promote write_build_config # creates a header of build-specific config info.

  popd &>/dev/null
fi

# build a few other utilities.
if [ ! -f "$BINARY_DIR/short_path$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/sleep_ms$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/create_guid$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/zap_process$EXE_ENDING" \
    -o ! -f "$BINARY_DIR/playsound$EXE_ENDING" ]; then
  pushd "$TOOL_SOURCES/simple_utilities" &>/dev/null
  make_code pre_compilation OMIT_VERSIONS=t
  make_code OMIT_VERSIONS=t

  promote create_guid  # globally unique ID creator.
  promote playsound  # sound playback tool.
  promote short_path  # provides short path names for exes on windows.
  promote sleep_ms  # sleep tool is used in some scripts.
  promote zap_process  # kills a process in the task list.

  popd &>/dev/null
fi

echo "The build binaries have been re-created (or were already present)."

# we won't do the full build if they told us to just do the bootstrap.
if [ -z "$JUST_BOOTSTRAP_APPS" ]; then
  echo Cleaning up the temporary files that were built.
  bash "$BUILD_SCRIPTS_DIR/whack_build.sh" clean

  # recreate our useful junk directories...
  mkdir -p "$WASTE_DIR"
  mkdir -p "$TEMPORARIES_DIR"
  mkdir -p "$LOGS_DIR"

  echo Now starting a normal build of the repository source code.
  pushd "$REPOSITORY_DIR" &>/dev/null
  unset BUILD_DEFAULTS
  declare -a BUILD_DEFAULTS=( "BOOT_STRAPPING=" "OPTIMIZE=t" "DEBUG=t" "REBUILD=t" )
  export NOT_SLIMLINE=true
  make_code
  popd &>/dev/null

echo hey ho ho hey, got to final place
ls -al $PRODUCTION_DIR/logs

fi

