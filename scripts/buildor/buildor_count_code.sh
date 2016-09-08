#!/bin/bash

TOTALS=0

codefile_list=(.c .cpp .h .java .pl .py .sh .wsdl .gwsdl .xml .properties .config .configuration .txt makefile\* )

for ((i=0 ; i < ${#codefile_list[@]}; i++)); do
  if [ ! -z "$phrases" ]; then
    phrases="$phrases -o"
  fi
  phrases="$phrases -iname \*${codefile_list[i]}"
done
#echo phrases is now $phrases

while true; do
  export NAME_LIST_TEMP_FILE="$(mktemp "$TMP/zz_code_count.XXXXXX")"

  dir="$1"; shift
  if [ -z "$dir" ]; then
    break;
  fi
#echo dir is $dir

  # for some reason we had to add an eval in front to get this
  # working after escaping the asterisk (so as not to include
  # local files that matched the patterns).
  eval find "$dir" -type f $phrases >$NAME_LIST_TEMP_FILE

#echo ====================================================
#echo file $NAME_LIST_TEMP_FILE holds these matches:
#cat $NAME_LIST_TEMP_FILE
#echo ====================================================

  while read line; do
#echo line is $line
    count=$(grep -h -v -c "^[ ]*$" $line)
    TOTALS=$(($TOTALS + $count))
#echo total is $TOTALS now
  done < "$NAME_LIST_TEMP_FILE"
  
  rm $NAME_LIST_TEMP_FILE

done

echo "total lines of code=$TOTALS"

