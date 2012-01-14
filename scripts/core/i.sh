#!/bin/bash

(
source $SHELLDIR/core/date_stringer.sh

echo '###############################################################################'
echo
echo Your user name is $USER on a computer named $(hostname).
echo Your machine platform is $(uname -m).
echo The time is $(date_stringer | sed -e 's/_/ /g' | sed -e 's/\([0-9][0-9]\) \([0-9][0-9]\)$/:\1:\2/')
echo
echo '###############################################################################'
echo
echo You have the following files here:
ls -FC
echo
echo '###############################################################################'
echo
echo You are sharing the machine with:
who
echo
echo '###############################################################################'
echo
echo You are running the following processes:
. $SHELLDIR/users/findme.sh
echo
echo '###############################################################################'
) | less
