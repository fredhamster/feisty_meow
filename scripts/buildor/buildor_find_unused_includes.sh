#!/bin/bash

# looks for include files that have been commented out.

tab=$'\t'
find $* -type f -exec grep -l "^[ $tab_char]*\/\/[ $tab_char]*#include" {} ';' | grep -v ".*\.svn.*"

