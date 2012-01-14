#!/bin/bash
COMMAND='play -v 10'

COUNT=$#
if [ $COUNT -ne 1 -o ! -d $1 ]; then
	echo random_iter needs a directory name where the files to be randomly
	echo chosen from are located.
	\exit 0
fi

NUMBER_OF_FILES=$(ls -1 $1/* | wc | awk '{ print $1; }')

if [ $NUMBER_OF_FILES -lt 1 ]; then
	echo There are no files in $1.
	\exit 0
fi

RANDOM_CHOICE=$(expr $RANDOM % $NUMBER_OF_FILES + 1)
CURRENT_PLACE=1
for i in $1/*; do
  if [ $CURRENT_PLACE = $RANDOM_CHOICE ]; then
    $COMMAND $i
    \exit 0
  fi
  CURRENT_PLACE=$(expr $CURRENT_PLACE + 1)
done
