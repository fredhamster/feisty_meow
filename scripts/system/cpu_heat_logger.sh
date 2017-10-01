#!/bin/bash

# queries the cpu temperature using the sensors system (must be installed) and writes the
# results to an ongoing log file as well as standard output.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

(
  while true; do
    sleep 1m 
    date
    sensors
    sep
  done
) | tee -a ${TMP}/cpu_temp.log 


