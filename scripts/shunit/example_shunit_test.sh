#!/bin/bash

# An example of using shunit2.
#
# Author: Chris Koeritz

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
#cd $WORKDIR
if [[ ! "$0" =~ ^/.* ]]; then
  # re-run the script with the absolute path if it didn't start that way; otherwise
  # shunit is not happy with finding the script.
  exec "$WORKDIR/$(basename $0)" $*
fi

oneTimeSetUp()
{
  echo "into oneTimeSetUp."
}

testOneThing()
{
  echo "got to test case.  sleeping for a bit..."
  zero=0
  assertEquals "zero should be equal to 0" 0 $zero
  sleep 23
  echo "woke up.  passed tautological test."
}

oneTimeTearDown()
{
  echo "into oneTimeTearDown."
}

# load and run shUnit2
source $SHUNIT_DIR/shunit2

