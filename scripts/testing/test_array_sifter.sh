#!/bin/bash
#
# tests the array sifter methods.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/core/array_sifter.sh"

#demo 1 & 2 for test presence.
declare -a my_array=(peanuts sauce fish basil)

test1=basil
test_presence my_array $test1
if [ $? != 0 ]; then
  echo "test1 did not find flag, but should have."
else
  echo "test1 found expected flag."
fi

test2=spoo
test_presence my_array $test2
if [ $? != 0 ]; then
  echo "test2 did not find flag, which is correct."
else
  echo "test2 found flag when should not have."
fi

#############################################

#demo3 for sifting...

declare -a sift_list=(ontrack selenium aggressive)
declare -a stripping=(selenium)
declare -a store_list=()

sift_array "sift_list" "stripping" "store_list" 
if [ ${store_list[0]} == "selenium" ]; then
  echo "test3 found expected content in storage list."
else
  echo "test3 was missing expected content in storage list."
fi
echo sift is now ${sift_list[*]}
if [ "${sift_list[*]}" == "ontrack aggressive" ]; then
  echo "test3 found expected content in sifting list."
else
  echo "test3 was missing expected content in sifting list."
fi


