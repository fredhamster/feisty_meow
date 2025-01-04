#!/usr/bin/env bash

# This script runs through all the known tests by performing a test sweep.
# It should claim that all tests succeeded for a new build/configuration/etc.
# to be considered successful.
#
# The way to add tests to the full test set is to add full paths to the
# "TESTKIT_TEST_SUITE" variable.  This can be done in your personal
# testkit.config file or in an exported variable set prior to running
# this script.
#
# Author: Chris Koeritz

export TESTKIT_DIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd $TESTKIT_DIR

TIME_START="$(date +"%s")"

source prepare_tools.sh prepare_tools.sh

# if that didn't work, complain.
if [ -z "$TESTKIT_SENTINEL" ]; then echo Please run prepare_tools.sh before testing.; exit 3; fi
source "$TESTKIT_ROOT/library/establish_environment.sh"

verbosity="$1"; shift

VERBOSE=1

if [ "$verbosity" == "--help" -o "$verbosity" == "-help" -o "$verbosity" == "-h" ]; then
  echo "$(basename $0): Runs the available suite of tests."
  echo
  echo "   $(basename $0) {summary | [full]}"
  echo
  echo "By default, the report will be a 'full' listing that includes all test"
  echo "run logging.  If 'summary' is passed as the first parameter, then only"
  echo "the test results will be displayed."
  exit 0
fi

if [ "$verbosity" == "summary" ]; then
  VERBOSE=0
fi

##############

# clean up any conglomerated log file.
rm -f "$CONGLOMERATED_TESTKIT_OUTPUT"

##############

# define the sets of tests we'd like to run.

NETBADGE_TESTS=( \
  netbadge_integrations/basic_integrations_test.sh \
)

##############

if [ ! -z "$AUTOBUILD_RUNNING" ]; then
  # only add some tests for automated, testing, bootstrap builds based on their needs.

true  # placeholder

fi

##############

# now that all tests have been defined, we build up our total list of tests.

echo Full set of tests:
for ((test_iter=0; $test_iter < ${#TESTKIT_TEST_SUITE[*]}; test_iter++)); do
  echo "$(expr $test_iter + 1): ${TESTKIT_TEST_SUITE[$test_iter]}"
done

##############

FAIL_COUNT=0

REG_TEMP="$TEST_TEMP/run_$(date +"%Y_%m_%d")"
if [ ! -d "$REG_TEMP" ]; then
  mkdir "$REG_TEMP"
fi

# go to the top of the hierarchy.
cd "$TESTKIT_ROOT"

for ((test_iter=0; $test_iter < ${#TESTKIT_TEST_SUITE[*]}; test_iter++)); do
  echo -e "\n======================================================================"
  echo -n `date`": " 
  echo "Now running test $(expr $test_iter + 1): ${TESTKIT_TEST_SUITE[$test_iter]}"
  output_file="$(mktemp $REG_TEMP/test_log.XXXXXX)"
  echo "  Test output file: $output_file"

#
#hmmm: no real way to check for errors in the general case, unless we define a
#      set of sentinels for this.  not done yet.
#-->
#  echo "==============" >"$output_file"
#  echo "Log state prior to test:" >>"$output_file"
#  check_logs_for_errors >>"$output_file"
#  echo "==============" >>"$output_file"

  if [ $VERBOSE -ne 1 ]; then
    bash "${TESTKIT_TEST_SUITE[$test_iter]}" >>"$output_file" 2>&1
    retval=$?
  else
    bash "${TESTKIT_TEST_SUITE[$test_iter]}" 2>&1 | tee -a "$output_file"
    retval=${PIPESTATUS[0]}
  fi

  if [ $retval -ne 0 ]; then
    ((FAIL_COUNT++))
    echo "FAILURE: exit code $retval for test ${TESTKIT_TEST_SUITE[$test_iter]}"
    TEST_RESULTS[$test_iter]="FAIL"
  else
    echo "OK: successful test run for test ${TESTKIT_TEST_SUITE[$test_iter]}"
    TEST_RESULTS[$test_iter]="OKAY"
  fi

#hmmm: same comment re error checking...  define some tags to look for!
#  echo "==============" >>"$output_file"
#  echo "Log state after test:" >>"$output_file"
#  check_logs_for_errors >>"$output_file"
#  echo "==============" >>"$output_file"

done

# final analysis--how did the test run do?

echo -e "\n\nResults table for this test run:\n"
for ((test_iter=0; $test_iter < ${#TESTKIT_TEST_SUITE[*]}; test_iter++)); do
  num=$(expr $test_iter + 1)
  if [ $num -lt 10 ]; then num="0$num"; fi
  echo "$num: ${TEST_RESULTS[$test_iter]} -- ${TESTKIT_TEST_SUITE[$test_iter]}"
done
echo

# figure out how long things took.
TIME_END="$(date +"%s")"
duration="$(($TIME_END - $TIME_START))"
# prepare to print duration in hours and minutes.
minutes="$(($duration / 60))"
hours="$(($minutes / 60))"
# grab out the hours we calculated from the minutes sum.
minutes="$(($minutes - $hours * 60))"
if (($minutes < 10)); then minutes="0$minutes"; fi
if (($hours < 10)); then hours="0$hours"; fi
echo "Total testing duration: $hours:$minutes hh:mm ($duration seconds total)"

if [ $FAIL_COUNT -ne 0 ]; then
  echo "FAILURE: $FAIL_COUNT Tests Failed out of ${#TESTKIT_TEST_SUITE[*]} Tests."
  exit 1
else
  echo "OK: All ${#TESTKIT_TEST_SUITE[*]} Tests Ran Successfully."
  exit 0
fi


