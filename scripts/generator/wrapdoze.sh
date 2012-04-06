#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

declare -a commands=()

for i in "$@"; do
  # we only mess with the command line on windows.
  if [ "$OS" == "Windows_NT" ]; then
    commands+=($(msys_to_dos_path $i))
  else
    commands+=("$i")
  fi
done

#  echo commands are now:
#  for i in "${commands[@]}"; do
#    echo $i
#  done

# now actually run the possibly chewed command.
"${commands[@]}"


