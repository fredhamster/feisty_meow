#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

#hmmm: make this support multiple vars as parameters.
# replaces a specific environment variable with a dos approved equivalent.
function dossify_environment_variable()
{
  local var="$1"; shift

#cygpath doesn't handle multiple path variables properly and otherwise operates only on one path element.
##  new_value="$(cygpath -p -d ${!var})"
##  eval "export $var=\"$new_value\""
##echo "hey now the var is '${!var}'"

  old_value="${!var}"
#echo "var is '$var' and old value is '$old_value'"
  if [[ ! "$old_value" =~ \/cygdrive\/ ]]; then
#echo didnt have a cygdrive in it: $old_value
    return 0
  fi

  # replace single back-slashes with double ones.
  local new_value="$(unix_to_dos_path "${old_value}")"

  # remove any quote characters in the value.
  new_value="${new_value//\"/}"

echo "new value: '$var' = '$new_value'"
  eval "export $var=\"$new_value\""
}

# for a windows build, this will replace any forward slashes
# and other cygwin notation with the appropriate dos style paths.
function dossify_and_run_commands()
{
  if [ "$OS" != "Windows_NT" ]; then
    # for non windows, just run the commands straight up.
    eval "${@}"
    return $?
  fi

  # force all slashes to be dossy.
#  export SERIOUS_SLASH_TREATMENT=true

  dossify_environment_variable INCLUDE

  declare -a darc_commands=()

  for i in "$@"; do
    if [[ "$i" =~ ^-[a-zA-z][/\"].* ]]; then
      flag="${i:0:2}"
      filename="$(unix_to_dos_path ${i:2})"
#echo "first two chars are $flag"
#echo "last after that are $filename"
      recombined="$flag$filename"
#echo combined flag and file is $recombined
      darc_commands+=("$recombined")
    elif [[ "$i" =~ ^-libpath:.* ]]; then
      flag="-libpath:"
      filename="$(unix_to_dos_path ${i:9})"
#echo "libflag flag is $flag"
#echo "name after that is $filename"
      recombined="$flag$filename"
#echo combined flag and file is $recombined
      darc_commands+=("$recombined")
    else 
      darc_commands+=($(unix_to_dos_path $i))
    fi
  done

  declare -a real_commands=()
  for i in "${darc_commands[@]}"; do
    real_commands+=($(echo $i | sed -e 's/\//\\/g'))
  done

  if [ ! -z "$SHELL_DEBUG" ]; then
    echo commands are now:
    for i in "${real_commands[@]}"; do
      echo -n "$i "
    done
    echo
  fi

# this nonsense is only necessary because cygwin is screwing up our carefully constructed
# command line.  it can't seem to leave the dos style paths alone in some cases, and insists
# on changing their form to use forward slashes, which breaks the windows compiler.
# this is NOT what cygwin is supposed to be doing, according to their documentation that
# claims all styles of paths are supported.  and of course this worked fine in msys.

  # now actually run the chewed command.

# old approach, not working since cygwin is hosing us on some paths.
#cmd /c "${real_commands[@]}"

#new approach that creates a cmd file.
  cmdfile="$(mktemp $CLAM_TMP/build_cmds.XXXXXX)"
  echo "${real_commands[@]}" >"$cmdfile"
echo "** cmd file is: '$cmdfile')"
#echo "** cmd file has: $(cat "$cmdfile")"
  cmd /c $(cat "$cmdfile")
  retval=$?
  # leave the file for inspection if there was an error.
  if [ $retval -eq 0 ]; then
    \rm "$cmdfile"
  fi
  return $retval
}

dossify_and_run_commands "$@"

