#!/bin/bash

main_pid=$( ps wuax | grep "[0-9] mono OpenSim.exe" | grep -vi screen | sed -e "s/$USER  *\([0-9][0-9]*\).*/\1/" )

if [ ! -z "$main_pid" ]; then
  echo Zapping main opensim process with id $main_pid.
  kill -9 $main_pid
else
  echo There does not seem to be a main opensim process running.
fi


