#!/bin/bash

function spacem_out()
{
  while [ $# -gt 0 ]; do
    arg="$1"; shift
echo "arg is '$arg'"
    if [ ! -f "$arg" -a ! -d "$arg" ]; then
      echo "=> did not find a file or directory named '$arg'."
      continue
    fi

    # we capture the output of the character replacement operation for reporting.
    # this is done first since some filenames cannot be properly renamed in perl (e.g. if they
    # have pipe characters apparently).
    intermediate_name="$(bash "$FEISTY_MEOW_SCRIPTS/files/replace_spaces_with_underscores.sh" "$arg")"
    local saw_intermediate_result=0
    if [ -z "$intermediate_name" ]; then
echo no new intermed name reported
      # make sure we report something, if there are no further name changes.
      intermediate_name="'$arg'"
    else 
      # now zap the first part of the name off (since original name is not needed).
      intermediate_name="$(echo $intermediate_name | sed -e 's/.*=> //')"
      saw_intermediate_result=1
    fi

    # here we rename the file to be lower case.
    actual_file="$(echo $intermediate_name | sed -e "s/'\([^']*\)'/\1/")"
echo actual file computed: $actual_file
    final_name="$(perl "$FEISTY_MEOW_SCRIPTS/files/renlower.pl" "$actual_file")"
    local saw_final_result=0
echo temp final name is: $final_name
    if [ -z "$final_name" ]; then
      final_name="$intermediate_name"
    else
      final_name="$(echo $final_name | sed -e 's/.*=> //' )"
      saw_final_result=1
    fi
echo intermed result=$saw_intermediate_result 
echo intermed name=$intermediate_name
echo final result=$saw_final_result 
echo final name=$final_name

    if [[ $saw_intermediate_result != 0 || $saw_final_result != 0 ]]; then
      # printout the combined operation results.
      echo "'$arg' => $final_name"
    fi
  done
}

# this block should execute when the script is actually run, rather
# than when it is just being sourced.
if [[ $0 =~ .*spacem\.sh.* ]]; then
echo inside exec block for spacem
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  spacem_out "${@}"
  exit_on_error "running spacem_out on a list: ${@}"
fi

