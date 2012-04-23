#!/bin/bash
##############
# Name   : target_runner.sh
# Author : Chris Koeritz
# Rights : Copyright (C) 2012-$now by Feisty Meow Concerns, Ltd.
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############
#
# Runs the programs listed in clam's RUN_TARGETS and reports any errors seen.

#echo entering target_runner with variables:
#echo -e "\tRUN_TARGETS=${RUN_TARGETS}"
#echo -e "\tDIRTY_FILE=${DIRTY_FILE}"
#echo -e "\tSUBMAKE_FLAG=${SUBMAKE_FLAG}"
#echo -e "\tFAILURE_FILE=${FAILURE_FILE}"

if [ ! -z "${RUN_TARGETS}" ]; then
  if [ -f "${DIRTY_FILE}" -o -f "${SUBMAKE_FLAG}" ]; then
    total_exitval=0;
    for program_name in ${RUN_TARGETS}; do
      base=$(basename $program_name);
      "$program_name";
      exitval=$?;
      if [ $exitval -ne 0 ]; then
        echo "ERROR: $program_name exits with $exitval at $(date)";
        total_exitval=$(($total_exitval + 1));
      fi;
    done;
    if [ $total_exitval -ne 0 ]; then
      echo "FAILURE: $total_exitval errors occurred in RUN_TARGETS.";
      echo yep >"${FAILURE_FILE}";
      exit 1;
    fi;
  fi;
fi

