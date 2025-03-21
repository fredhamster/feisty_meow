#!/usr/bin/env bash

# an unfinished idea for how to manage identifiers that are randomly
# generated but are also uniquely identified for later use.
#
# Author: Chris Koeritz

export RANDOM_IDS=()

# given a name for a randomly assigned number, this will generate a
# new random value and add it to the RANDOM_IDS list.  it is an error
# to try to change an existing random id, because the id may already have
# been used in file generation and so forth.
function setup_random_id()
{
  name="$1"; shift
  if [ ! -z "${RANDOM_IDS[$name]}" ]; then
    echo "FAILURE: trying to reassign already generated random id for '$name'"
    return 1
  fi
  new_id=$RANDOM-$RANDOM-$RANDOM
  RANDOM_IDS[$name]=$new_id
  return 0
}

# returns the random value assigned under the name.  it is an error
# to request one that does not exist yet; this implies the test has
# not properly configured the random ids it will use.
function get_random_id()
{
  name="$1"; shift
  if [ -z "${RANDOM_IDS[$name]}" ]; then
    echo "FAILURE-to-find-$name"
    return 1
  fi
  echo "${RANDOM_IDS[$name]}"
  return 0
}

## 
## # test suite
## setup_random_id "george"
## if [ $? -ne 0 ]; then echo TEST failed to set george; fi
## setup_random_id "lucy"
## if [ $? -ne 0 ]; then echo TEST failed to set lucy; fi
## echo "lucy's id is: $(get_random_id lucy)" 
## lucy_id=$(get_random_id lucy)=
## if [ $? -ne 0 ]; then echo TEST failed to get lucy; fi
## echo "george's id is: $(get_random_id george)"
## george_id=$(get_random_id george)
## if [ $? -ne 0 ]; then echo TEST failed to get george; fi
## 
## setup_random_id "george" &>/dev/null
## if [ $? -eq 0 ]; then echo TEST failed to trap george being reset; fi
## setup_random_id "lucy" &>/dev/null
## if [ $? -eq 0 ]; then echo TEST failed to trap lucy being reset; fi
## get_random_id tony
## if [ $? -eq 0 ]; then echo TEST failed to trap non-existent id request; fi
## 


