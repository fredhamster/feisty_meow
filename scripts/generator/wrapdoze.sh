#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

function dossify_and_run_commands()
{
  # we only mess with the command line on windows...
  if [ "$OS" == "Windows_NT" ]; then
    # for non windows, just run the commands straight up.
    $*
    return $?
  fi

  declare -a darc_commands=()

  for i in "$@"; do
    if [[ "$i" =~ ^-[a-zA-z][/\"].* ]]; then
echo found parameter to fix...
      flag="${i:0:2}"
      filename="$(unix_to_dos_path ${i:2})"
echo "first two chars are $flag"
echo "last after that are $filename"
      recombined="$flag$filename"
echo combined flag and file is $recombined
      darc_commands+=("$recombined")
    else 
      darc_commands+=($(unix_to_dos_path $i))
    fi
  done

  declare -a real_commands=()
  for i in "${darc_commands[@]}"; do
    real_commands+=($(echo $i | sed -e 's/\\/\\\\/g'))
  done

#temp!
  echo commands are now: >>/tmp/wrapdoze.log
  for i in "${real_commands[@]}"; do
    echo $i >>/tmp/wrapdoze.log
  done
#end temp

  # now actually run the chewed command.
  cmd /c "${real_commands[@]}"
}

dossify_and_run_commands "$@"

