#!/usr/bin/env bash

# Supports the TestKit with a few handy functions and many variables.
#
# Author: Chris Koeritz

##############

# pull in the really basic functions...
source "$TESTKIT_ROOT/library/helper_methods.sh"

##############

# this check should be called first, in oneTimeSetUp, in every test script that uses shunit.
# it will make sure that important facts are true about the test environment.
#
#hmmm: need to extend this to allow them to add their own sanity checks to it,
#      similarly to how we need to add log parsing capability as an extension.
#
function sanity_test_and_init()
{
  if [ -z "$WORKDIR" ]; then
    echo "The WORKDIR variable is not set.  This should be established by each test, near the top."
    exit 1
  fi
  # establish this for shunit so tests do not have to run in current directory.
  export SHUNIT_PARENT="$WORKDIR/$(basename "$0")"

#hmmm: add other checks here, including the user provided ones.

  return 0
}

##############

# this is the main source of parameters for the tests.
export TESTKIT_CFG_FILE
if [ ! -f "$TESTKIT_CFG_FILE" ]; then
  # well, no config file variable defined, so we go with our default.
  # this file must exist or we're stumped.
  TESTKIT_CFG_FILE="$TESTKIT_ROOT/testkit.config"
fi
if [ ! -f "$TESTKIT_CFG_FILE" -a -z "$BADNESS" ]; then
  echo "----"
  echo "This script requires that you prepare a customized file in:"
  echo "    $TESTKIT_CFG_FILE"
  echo "    (above is current value of TESTKIT_CFG_FILE variable)"
  echo "with the details of your testing configuration."
  echo "There are some example config files in the folder:"
  echo "    $TESTKIT_ROOT/examples"
  BADNESS=true
fi

##############

# make sure we aren't premature in processing the config file.
if [ -z "$TESTKIT_BOOTSTRAPPING" ]; then

  # read the config file and generate environment variables for all the entries.
  source "$TESTKIT_ROOT/library/process_configuration.sh"
  define_and_export_variables
  check_if_failed "Not all variables could be imported properly from the configuration file '$TESTKIT_CFG_FILE'"

fi

##############

# announce status if in debugging mode.

if [ ! -z "$DEBUGGING" -a -z "$SHOWED_SETTINGS_ALREADY" ]; then
  echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  echo TestKit running from: $TESTKIT_ROOT
  echo TestKit config file: $TESTKIT_CFG_FILE
  echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
fi

##############

# more steps we're not ready for if still bootstrapping our environment.
if [ -z "$TESTKIT_BOOTSTRAPPING" ]; then
  # try to not blast out the above block of info again during this run.
  export SHOWED_SETTINGS_ALREADY=true

  # now that we have the environment set up, we can pull in all the functions
  # we use for running the tests...
  source "$TESTKIT_ROOT/library/runner_functions.sh"
  source "$TESTKIT_ROOT/library/random_ids_manager.sh"
  #hmmm: random ids are not used yet, are they?  are they working?
  source "$TESTKIT_ROOT/library/file_management.sh"

fi

##############

