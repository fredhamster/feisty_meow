#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
date
echo
echo zapping all the mono tasks...
procs=$(psfind mono)
echo processes marked for death: $procs
for i in $procs; do
  kill $i
done
sep
echo here are the mono processes after zapping:
psa mono
sep


