#!/bin/bash

##############

# simple examples...

declare -A snuggles
  # make an associative array

snuggles=([book]=petunia [muffets]="glasgow robbery")
  # keys: book and muffets
  # values: (second part)

snuggles+=([morgower]=flimshaw)
  # adding entries to it.

echo ${!snuggles[x]}
  # show index x's key.

echo ${snuggles[x]}
  # show index x's value.

##############

# excellent code from:
# http://blog.spencertipping.com/2009/08/constant-time-associative-arrays-in-bash
 
typeset -a table_keys
typeset -a table_values
 
function index_of_key () {
  initial=$(($(echo $1 | md5sum | cut -c 18-32 | awk '{print "0x"$1}')))
  while [[ ${table_keys[$initial]} && ${table_keys[$initial]} != $1 ]]; do
    initial=$((initial + 1))
  done
  echo -n $initial
}
 
function associate () {
  index=$(index_of_key $1)
  table_keys[$index]=$1
  table_values[$index]=$2
  echo -n $2
}
 
function lookup () {
  index=$(index_of_key $1)
  echo -n ${table_values[$index]}
}
 
echo Associating foo with bar and bif with baz
associate foo bar && echo
associate bif baz && echo
 
echo -n Looking up foo:
lookup foo && echo
 
echo -n Looking up bif:
lookup bif && echo
 
echo -n Looking up bar:
lookup bar && echo

##############

