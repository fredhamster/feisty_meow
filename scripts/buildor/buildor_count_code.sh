#!/bin/bash

export OUT="$(mktemp "$TMP/zz_code_count.XXXXXX")"

find $* -iname "*.cpp" -o -iname "*.h" -o -iname "*.c" >$OUT

TOTALS=0
for i in $(cat $OUT); do
  count=$(grep -h -v -c "^[ ]*$" $i)
  TOTALS=$(expr $TOTALS + $count)
#hmmm: is there a better way to do the addition?
done

rm $OUT

echo total lines of code=$TOTALS

