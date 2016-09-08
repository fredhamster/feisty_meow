#!/bin/bash
############################################################################
# This function parses a line from a CSV file (you pass the line as the 
# argument) into elements in an array.  The CSV elements must be enclosed
# in double-quotes and then separated by a comma.
#
# Commas are replaced by tildes as the separator character in an attempt to
# allow elements to contain commas.  If elements also contain tildes, a new
# separation character can be substituted by setting the variable
# UNIQUE_SEPARATOR to the value of that new separator character.
#
# Right now double-quote characters are also removed from the final output,
# so if one of you elements contains double-quotes this function will remove
# those double-quotes from within your element.
#
# Author: Chris Koeritz
############################################################################

declare -a csv_split=()

# you can override the chosen separator if your data has tildes in it...
if [ -z "$UNIQUE_SEPARATOR" ]; then
  UNIQUE_SEPARATOR='~'
fi

# parses a line of CSV text and turns it into an array called "csv_split".
# one defect of this approach is that if there are occurrences of the separator
# character in the middle of the quoted strings, they will not be handled
# properly.
function parse_csv_line()
{
  local parm="$1"; shift
#echo line before is: $parm
  csv_split=()
  # fix the line so we don't mistake embedded commas as separators.
  to_split="$(echo "$parm" | sed -e "s/\" *, *\"/\"$UNIQUE_SEPARATOR\"/g")"
#echo line afterwards is: $to_split
  # swap the IFS so we can find the breaks.
  OLD_IFS="$IFS"
  IFS="$UNIQUE_SEPARATOR"
  local csv_temp=($to_split)
  IFS="$OLD_IFS"
  # loop through and strip out the quotes.
  i=0
  while [ $i -lt ${#csv_temp[*]} ]; do
    csv_split[$i]="$(echo ${csv_temp[$i]} | sed -e 's/"//g')"
    i=$((i+1))
  done
}

############################################################################
