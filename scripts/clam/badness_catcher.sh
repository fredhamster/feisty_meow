#!/usr/bin/env bash
# badness_catcher: runs the command line passed in and catches error conditions.

if [ ! -z "$NOISY" ]; then
  echo $*
fi
eval "$@"

# get exit status.
ERR=$?

if [ $ERR -eq 0 ]; then exit; fi  # exit if no error.

# print a complaint since there was an error.
echo
echo "======================="
echo
echo "Error in project: \"$PROJECT\""
echo "  command was: $*"
echo
echo "======================="
source $CLAM_SCRIPTS/exit_make.sh
exit $ERR

