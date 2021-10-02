#!/bin/bash

# Test: Blank
# Author: Fill ItIn

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd "$WORKDIR"

# this needs to be relative to where the test will actually reside; the .. components below
# need to get to the top of the tools and tests hierarchy.
relative_depth=".."
source "$relative_depth/prepare_tools.sh" "$relative_depth/prepare_tools.sh"
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
  # take steps to clean last test, if any are needed.
  true
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

testShunitExamples()
{
  echo running through shunit tests available.

  # values must be the same.
  assertEquals "equals test" 5 5

  # similar test, different name.
  assertSame "same test" oof oof

  # values must not be the same.
  assertNotEquals "not equals test" 3 5

  # a parallel not same test.
  assertNotSame "not same test" orp 3
  
  # value must have no content (or not be defined).
  assertNull "empty null test" ""
  assertNull "undefined null test" "$variableDoesntExistOrShouldntSinceWeDidntDefineIt"

  # value must have content.
  assertNotNull "not null test" "sugarwater"

  # value must be true, which in bash means equal to zero.
  assertTrue "true test simple" 0
  # this shows how true return value can be tested.
  true
  assertTrue "true test return value" $?

  # value must be false, or non-zero.
  assertFalse "false test simple" 13
  # shows how false return value can be tested.
  false
  assertFalse "false test" $?
}

testShowSkipping()
{
  # the failures below are intentionally skipped, so as to avoid the failure
  # results in our full output.
  # this shows two things; how to skip the rest of the tests if needed,
  # and how to cause a test failure without using an assertion.
  
  # show if we're skipping already.
  isSkipping
  if [ $? -eq 0 ]; then
    echo we are skipping tests.
  else
    echo we are not skipping tests.
  fi

  # start skipping tests.
  startSkipping

  # show that we're skipping.
  isSkipping
  if [ $? -eq 0 ]; then
    echo we are skipping tests.
  else
    echo we are not skipping tests.
  fi

  # can be used to set a test failed state, without using an assertion.
  # this might be used if there is complex logic and one doesn't feel
  # like boiling that down to a single variable to test on.
  fail "one of the tests above failed"

  # indicates a failure because two values were not the same.
  failNotSame "values should have been the same" toyBoat oatBoy

  # indicates a failure when two values should have been different.
  failSame "values should not have been the same" 3 3

  # stop skipping tests again.
  endSkipping

  # show that we're no longer skipping.
  isSkipping
  if [ $? -eq 0 ]; then
    echo we are skipping tests.
  else
    echo we are not skipping tests.
  fi

  assertSame "shunit is cool" cool cool
}

oneTimeTearDown() {
  # cleaning up after test, if needed.
  true
}

# load and run shUnit2
source "$SHUNIT_DIR/shunit2"

