#!/usr/bin/env bash

# Author: Chris Koeritz
#
# Note:
# We do not want to "exit" from this file at all (nor from any file that it
# invokes either), since this script is intended for use by the bash 'source'
# command.  If we exit, that will exit from the calling shell as well, which
# torpedoes whatever one was doing in that shell.
# There is a variable below called BADNESS that indicates when errors
# occurred during processing, and if it's not empty at the end of the script
# then we will consider this a failed run, and we will not set the test's
# sentinel variable which other scripts check to see if the environment
# was loaded properly.

# make sure whether they have defined the top-level location for us.
if [ ! -z "$1" ]; then
  # first attempt is to use the first parameter, if one is provided.  this should
  # be an absolute path reference to this very file, from which we can deduce the
  # starting directory.
  GRITTY_TESTING_TOP_LEVEL="$( cd "$( dirname "$1" )" && \pwd )"
  # for this case, they also don't need to be stranded in a new shell, because we
  # assume they have sourced this file instead of bashing it.
  NO_SUBSHELL=true
fi
if [ -z "$GRITTY_TESTING_TOP_LEVEL" ]; then
  # otherwise, if they didn't explicitly set the top-level directory, we will
  # do it using some unix trickery.
  if [[ "$0" =~ .*bash ]]; then
    echo "----"
    echo "This script was not launched properly with 'source'.  The script should"
    echo "be started like this: source prepare_tools.sh prepare_tools.sh"
    echo "The double entry is required for bash's source command to find the path."
    BADNESS=true
  fi
  GRITTY_TESTING_TOP_LEVEL="$( cd "$( dirname "$0" 2>/dev/null )" && \pwd )"
else
  # we assume they are managing this script more closely and do not need (or want) a bash sub-shell.
  NO_SUBSHELL=true
fi
GRITTY_TESTING_TOP_LEVEL="$(echo "$GRITTY_TESTING_TOP_LEVEL" | sed -e 's/\/cygdrive\/\(.\)/\1:/')"

# the top-level directory for tests, i.e. the root of testing hierarchy.
export TESTKIT_ROOT="$GRITTY_TESTING_TOP_LEVEL"

# location needed for shunit temporary files.
export TMPDIR="$HOME/.shunit-temp"
if [ ! -d "$TMPDIR" ]; then
  mkdir -p "$TMPDIR"
  if [ $? -ne 0 ]; then
    echo "Failure during creation of TMPDIR for shunit!"
    exit 1
  fi
fi

# a bit of a dance to not pull in code too early...
export TESTKIT_BOOTSTRAPPING=true
source "$TESTKIT_ROOT/library/establish_environment.sh"
unset TESTKIT_BOOTSTRAPPING
# done with dancing, ready to pull in anything else from testkit.

#source "$TESTKIT_ROOT/library/helper_methods.sh"

# where the shunit library resides.
export SHUNIT_DIR="$TESTKIT_ROOT/shunit"

# establish the TMP variable if it's not already set.
export TMP
if [ -z "$TMP" ]; then
  TMP="$HOME/tmp"
  if [ ! -d "$TMP" ]; then mkdir "$TMP"; fi
fi
TMP="$(echo "$TMP" | sed -e 's/\/cygdrive\/\(.\)/\1:/')"
if [ ! -d "$TMP" ]; then 
  echo "The TMP directory was set as $TMP but cannot be created or found."
  echo "If there is a file at that location, please move or delete it."
  exit 1
fi

##############

# commonly used environment variables...

# TEST_TEMP is a folder where we can generate a collection of junk files.
export TEST_TEMP="$TMP/testkit_logs_${USER}"
if [ ! -d "$TEST_TEMP" ]; then
  mkdir -p "$TEST_TEMP"
fi

# this variable points to the last output from a grid command.
export TESTKIT_OUTPUT_FILE="$TEST_TEMP/testkit_output.log"
export TESTKIT_TIMING_FILE="$TEST_TEMP/testkit_times.log"
export CONGLOMERATED_TESTKIT_OUTPUT="$TEST_TEMP/full_testkit_output.log"

##############

# uncomment this to enable extra output.
export DEBUGGING=true

##############

# turn this printout off in non-debugging mode or if the terminal setting
# seems to indicate that we're running in a login environment (where any
# echoing to standard out can screw up scp and sftp for that account).
if [ ! -z "$DEBUGGING" -a -z "$SHOWED_SETTINGS_ALREADY" \
    -a -z "$BADNESS" -a -z "$SILENT_RUNNING" -a "${TERM}" != "dumb" \
    -a -z "$PBS_ENVIRONMENT" ]; then
  echo "==========================================================="
  echo "Testkit environment loaded."
  var TESTKIT_ROOT TESTKIT_CFG_FILE TMP TEST_TEMP
  echo "==========================================================="
fi

if [ ! -z "$(uname -a | grep -i darwin)" -a -z "$BADNESS" ]; then
  # add in the mac binaries if this is darwin.
  export PATH="$TESTKIT_ROOT/bin/macosx:$PATH"
else
  # no change, but we want to make sure sub-shells inherit the path.
  export PATH="$PATH"
fi

if [ -z "$NO_SUBSHELL" -a -z "$BADNESS" ]; then
  # at this point we go into a new interactive shell, so as to ensure the
  # environment parameters stay right.
  # the self-location code at the top doesn't work properly if this file is
  # sourced into a current environment.
  bash
fi

if [ ! -z "$BADNESS" ]; then
  echo
  echo "----"
  echo "There were errors in setting up the xsede tests--see above messages."
  unset TESTKIT_SENTINEL TESTKIT_ROOT GRITTY_TESTING_TOP_LEVEL SHUNIT_DIR BADNESS
else
  # if things were successful, we can finally set our indicator for the scripts to check.
  export TESTKIT_SENTINEL=initialized
fi

