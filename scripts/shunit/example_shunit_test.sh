#!/bin/bash

# An example of using shunit2.
#
# Author: Chris Koeritz

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd $WORKDIR

# this variable needs to be set somewhere.  feisty meow scripts will set it.
#SHUNIT_DIR=/home/fred/shunit

oneTimeSetUp()
{
  echo into set up.
}

testOneThing()
{
  echo got to test case.
  zero=0
  assertEquals "zero should be equal to 0" 0 $zero
  sleep 23
}

oneTimeTearDown()
{
  echo into tear down.
}

# load and run shUnit2
source $SHUNIT_DIR/shunit2

