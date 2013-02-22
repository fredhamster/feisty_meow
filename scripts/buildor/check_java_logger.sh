#!/bin/bash

# looks for occurrences of getting a Log from the log factory.  when we find
# one, we make sure it is using the same class name as the compilation unit.

file="$1"; shift

if [ -z "$file" -o ! -f "$file" ]; then
  echo This script needs a filename to check for appropriate logger creation.
  echo Any file that has a Log based on a different class than itself will
  echo be reported.
  exit 1
fi

class_in_logger="$(sed -n -e 's/.*LogFactory.getLog( *\([^\.]*\)\.class *).*/\1/p' <"$file")"

#echo got class from logger of $class_in_logger

if [ -z "$class_in_logger" ]; then
  # we didn't find a log factory.
  exit 0
fi

base_of_class="$(basename "$file" | sed -e 's/\(.*\)\.java/\1/')"

#echo base of class is $base_of_class

if [ "$class_in_logger" != "$base_of_class" ]; then
  echo "$file"
  exit 1
fi


