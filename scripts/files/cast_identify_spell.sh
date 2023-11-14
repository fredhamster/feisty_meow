#!/bin/bash

# calls a verbose identify on each file passed to it.
# this should shake loose all the metadata and display it.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"


for file in "${@}"; do
  echo
  sep
  echo "file: '$file'"
  echo
  identify -verbose "$file"
  sep
  echo
done
