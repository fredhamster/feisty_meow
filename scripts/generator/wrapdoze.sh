#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

function dossify_and_run_commands()
{
  declare -a darc_commands=()

  for i in "$@"; do
    # we only mess with the command line on windows.
    if [ "$OS" == "Windows_NT" ]; then
      if [[ "$i" =~ ^-[a-zA-z][/\"].* ]]; then
#echo matched on our pattern for parameters
        flag="${i:0:2}"
        filename="$(unix_to_dos_path ${i:2})"

#echo "first two chars are $flag"
#echo "last after that are $filename"
#combined="$flag$filename"
#echo combined is $combined
      
        darc_commands+=("$flag$filename")
      else 
        darc_commands+=($(unix_to_dos_path $i))
      fi
    else
      darc_commands+=("$i")
    fi
  done

#temp!
  echo commands are now: >>/tmp/wrapdoze.log
  for i in "${darc_commands[@]}"; do
    echo $i >>/tmp/wrapdoze.log
  done
#end temp

  # now actually run the possibly chewed command.
  "${darc_commands[@]}"
}

dossify_and_run_commands "$@"

