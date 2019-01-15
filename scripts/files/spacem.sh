#!/bin/bash

function spacem_out()
{
  while [ $# -gt 0 ]; do
    arg="$1"; shift
    if [ ! -f "$arg" -a ! -d "$arg" ]; then
      echo "=> did not find a file or directory named '$arg'."
      continue
    fi

    # first we will capture the output of the character replacement operation for reporting.
    # this is done first since some filenames cannot be properly renamed in perl (e.g. if they
    # have pipe characters apparently).
    intermediate_name="$(bash "$FEISTY_MEOW_SCRIPTS/files/replace_spaces_with_underscores.sh" "$arg")"
    local saw_intermediate_result=0
    if [ -z "$intermediate_name" ]; then
      # make sure we report something, if there are no further name changes.
      intermediate_name="'$arg'"
    else 
      # now zap the first part of the name off (since original name is not needed).
      intermediate_name="$(echo $intermediate_name | sed -e 's/.*=> //')"
      saw_intermediate_result=1
    fi

    # first we rename the file to be lower case.
    actual_file="$(echo $intermediate_name | sed -e "s/'\([^']*\)'/\1/")"
    final_name="$(perl $FEISTY_MEOW_SCRIPTS/files/renlower.pl "$actual_file")"
    local saw_final_result=0
    if [ -z "$final_name" ]; then
      final_name="$intermediate_name"
    else
      final_name="$(echo $final_name | sed -e 's/.*=> //' )"
      saw_final_result=1
    fi
#echo intermed=$saw_intermediate_result 
#echo final=$saw_final_result 

    if [[ $saw_intermediate_result != 0 || $saw_final_result != 0 ]]; then
      # printout the combined operation results.
      echo "'$arg' => $final_name"
    fi
  done
}

# this block should execute when the script is actually run, rather
# than when it is just being sourced.
if [[ $0 =~ .*spacem\.sh.* ]]; then
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  spacem_out "${@}"
  exit_on_error "running spacem_out on a list: $*"
fi

