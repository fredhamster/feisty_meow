#!/bin/bash

# An example of using shunit2.
#
# Author: Chris Koeritz
# license gnu gpl v3

export WORKDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
if [[ ! "$0" =~ ^/.* ]]; then
  # re-run the script with an absolute path if it didn't start out that way; otherwise,
  # shunit is not happy with finding the script.
  exec "$WORKDIR/$(basename $0)" $*
fi
cd $WORKDIR

oneTimeSetUp()
{
  echo "into oneTimeSetUp."
}

testOneThing()
{
  echo "got to primary test case."
  zero=0
  assertEquals "zero should be equal to 0" 0 $zero
  echo "passed tautological test."
  sleep_time=83
  echo "$(date): now sleeping for $sleep_time seconds."
  sleep $sleep_time
  echo "$(date): woke up."
}

oneTimeTearDown()
{
  echo "into oneTimeTearDown."
}

# load and run shUnit2
source $SHUNIT_PATH/shunit2

