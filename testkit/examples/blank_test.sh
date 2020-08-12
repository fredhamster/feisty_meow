#!/bin/bash

# Test: X
# Author: Y

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd "$WORKDIR"

# this needs to be relative to where the test will actually reside; the ../../../../../etc
# should get to the top of the tools and tests hierarchy.
source "../prepare_tools.sh" "../prepare_tools.sh"
if [ -z "$TEST_TEMP" ]; then
  echo The TestKit could not be automatically located.
  exit 1
fi

if [ -z "$TESTKIT_SENTINEL" ]; then echo Please run prepare_tools.sh before testing.; exit 3; fi
source "$TESTKIT_ROOT/library/establish_environment.sh"

oneTimeSetUp()
{
  # a test environment initializer method called directly by shunit.
  # you can add your own code here that is needed before the test run starts.
  true
}

# this exact test should always be the first one in a test suite.
testSanity()
{
  # make sure the test environment is good.
  sanity_test_and_init
  assertEquals "sanity test" 0 $?
}

testCleaningPriorTestRun()
{
  echo taking steps to clean last test...
#if any.
}

testDoAThing()
{
  echo doing one thing
  assertEquals "doing that thing should work" 0 $?
}

testDoAnotherThing()
{
  echo doing another thing here
  assertEquals "doing that other thing should work" 0 $?

  echo "about to cause a failure, to test assertNotEquals..."
  false
  assertNotEquals "an explicit failure should be seen" 0 $?
}

oneTimeTearDown() {
  echo cleaning up after test now...
#if anything to do.
}

# load and run shUnit2
source "$SHUNIT_DIR/shunit2"

