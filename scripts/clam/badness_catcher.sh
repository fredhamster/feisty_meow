#!/bin/bash
# badness_catcher: runs the command line passed in and catches error conditions.
#echo args are: $*
eval $*
ERR=$?  # get exit status.
if [ $ERR -eq 0 ]; then exit; fi  # exit if no error.
# print a complaint if there was an error.
echo
echo "Error in project \"$PROJECT\"!"
echo "  command=\"$*\"."
echo
source $CLAM_DIR/exit_make.sh
exit $ERR
