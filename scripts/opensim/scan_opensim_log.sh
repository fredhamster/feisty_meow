#!/bin/bash

# an example for windows with the log located under program files.
#PROGFILES="c:/Program Files"
#if [ -d "$PROGFILES (x86)" ]; then
#  PROGFILES="$PROGFILES (x86)"
#fi
#echo progfiles is $PROGFILES

# standard case for us still is opensim log...
BASE_LOG_FILE="$HOME/opensim/bin/OpenSim.log"

OLDER_LOGS=
for ((i=20; i >= 1; i--)) do
  OLDER_LOGS+="$BASE_LOG_FILE.$i "
done

#echo older is $OLDER_LOGS

grep -i "error\|text node\| bad \|warn\|deadlock\|exception\|missing\|violation\|failed" $OLDER_LOGS "$BASE_LOG_FILE" 2>/dev/null

