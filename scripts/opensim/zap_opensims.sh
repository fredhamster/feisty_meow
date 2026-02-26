#!/usr/bin/env bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
date
echo
echo zapping all the dotnet tasks...
procs=$(psfind dotnet)
echo processes marked for death: $procs
for i in $procs; do
  kill $i
done
sep
echo here are the dotnet processes after zapping:
psa dotnet
sep


