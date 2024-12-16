#!/usr/bin/env bash

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
  oldIFS="$IFS"
  IFS="$UNIQUE_SEPARATOR"
  local csv_temp=($to_split)
  IFS="$oldIFS"
  # loop through and strip out the quotes.
  i=0
  while [ $i -lt ${#csv_temp[*]} ]; do
    csv_split[$i]="$(echo ${csv_temp[$i]} | sed -e 's/"//g')"
    i=$((i+1))
  done
}

############################################################################

# main script body:

for i in $*; do
  echo reading $i
  while read input_text; do
#    echo input_text is $input_text
    csv_split=()
    parse_csv_line "$input_text"

    if [ ${#csv_split[*]} -lt 1 ]; then
      echo did not see any fields in the split array.
    else
      echo line splits into ${#csv_split[*]} fields:
      i=0
      while [ $i -lt ${#csv_split[*]} ]; do
        quoteless="${csv_split[$i]}"
        echo "$i: $quoteless"
        i=$((i+1))
      done
    fi
  done < $i
done

