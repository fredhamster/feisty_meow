#!/usr/bin/env bash

# sample the first argument to make sure it's not empty.
# we can't know if the command is really valid or not, but it at least
# needs to not be empty.
test_to_run="$1"

if [ -z "$test_to_run" ]; then
  echo "
rounder: this script requires a test or program to run as the first parameter.

the parameter can be an arbitrarily complex bash command as long as it's
escaped properly.  for example, this command will check a directory size and
then sleep for 10 seconds:

  rounder  '\du -s ~/a_folder_i_watch; sleep 10'

rounder will run this check, snoozing in between checks, until the universe
ends or something intervenes.  note that without the sleep, the check would
occur very rapidly and spew output.
"
  exit 1
fi

trashdir="$(mktemp -d "$TMP/rounder.XXXXXX")"

echo "Running command: $*"
echo "Storing log files in: $trashdir"

round=0
while true; do
  round=$((round+1))
  echo ============================
  echo round $round commencing
  outputfile="$trashdir/run_round_${round}.log"
echo real cmd:
echo "${@}" 
  eval "${@}" 2>&1 | tee $outputfile
  if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo FAILURE IN RUN $round
    break
  fi
  echo round $round successful.
done 

