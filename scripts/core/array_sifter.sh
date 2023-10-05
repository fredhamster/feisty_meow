#!/usr/bin/env bash
#
# Provides functions for checking and sorting the contents of arrays.


#hmmm: these could all be beefed up by properly handling spaces in array
#      entries.  use "${blah[@]}" rather than '*' for getting all elems.


# given the name of an array as the first parameter, this signals
# success (return value zero) if the second parameter is found in the
# array.  failure (non-zero return) occurs if the item is missing.
function test_presence()
{
  array_name=$1; shift
  flag_to_find=$1; shift
  temp_name="${array_name}[*]"
  declare -a array_to_check=(${!temp_name})
#echo "array to check is: ${array_to_check[*]}"
  local i;
  for (( i=0; i < ${#array_to_check[*]}; i++ )); do
    if [ "${array_to_check[i]}" == "$flag_to_find" ]; then
#      echo "found the flag $flag_to_find"
      return 0
    fi
  done
#  echo "did not find the flag $flag_to_find"
  return 1
}

# takes two arrays of items and checks whether any of the members of the second
# array are present in the first array.  zero is returned when a match is found.
function match_any()
{
  match_within_name=$1; shift
  items_to_match_name=$1; shift
#  temp_name="${match_within}[*]"
#  declare -a match_within_array=(${!temp_name})
#echo "the list to match within is: ${match_within_array[*]}"
  temp_name="${items_to_match_name}[*]"
  declare -a items_to_match_array=(${!temp_name})
#echo "the items to match are: ${items_to_match_array[*]}"

  for onetag in ${items_to_match_array[*]}; do
#echo curr tag is $onetag 
    test_presence $match_within_name $onetag
    if [ $? -eq 0 ]; then return 0; fi  # got a match.
  done
  return 1  # no match found.
}

# sifts an array by finding matching items in a second array and storing
# those extracted items into a third array.  the original array is modified
# by removing those matched items.  the first parameter is the array to sift,
# the second is a list of items to sift out of the array, and the third is
# an array to fill with removed/matched items.  each of these is just a
# name, which will be indirectly referenced to get its contents.
# success (zero) is returned when any item to match was found in the array.
function sift_array()
{
  sift_name=$1; shift
  temp_name="${sift_name}[*]"
  declare -a array_to_sift=(${!temp_name})
  match_name=$1; shift
  temp_name="${match_name}[*]"
  declare -a match_items=(${!temp_name})
  store_name=$1; shift

  declare -a temp_store=()  # put our extracted items for the moment.

  # seek backwards through the array to sift, so we can destructively
  # rearrange it without adjusting loop index.
  sift_len=${#array_to_sift[*]}
  local i; local j;
  for (( i=$sift_len; i >= 0; i-- )); do
    # look through match items to see if they're in the sifting array.
    for (( j=0; j < ${#match_items[*]}; j++ )); do
      curname=${match_items[$j]}
      if [ "${array_to_sift[$i]}" == "$curname" ]; then
        # found a lucky participant; move it to the storage array.
        temp_store+=($curname)
#echo temp store now ${temp_store[*]}
        # remove this item from the containing array.
        ((after=$i+1))
        ((afterlen=$sift_len-$i))

        array_to_sift=(${array_to_sift[*]:0:$i} ${array_to_sift[*]:$after:$afterlen})
      fi
    done
  done

  # update our list for what was removed.
  eval ${sift_name}=\(${array_to_sift[*]}\)

  # now assign to the external arrays.
  eval ${store_name}=\(${temp_store[*]}\)

  if [ ${#temp_store} -ne 0 ]; then
    return 0
  else
    return 1
  fi
}

