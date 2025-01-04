#!/usr/bin/env bash

# redirects standard out and standard error output to temp files and runs all
# the parameters to this script as a command.
# if there is no error, then the files are just deleted.
# if there was an error, then the two output files are sent to standard out
# and standard error, and an additional explanatory error message is sent to
# standard error about the command that failed.

#echo "squelch args: $(printf -- "[%s] " "${@}")"

newout="$(mktemp /tmp/squelch.out.XXXXXX)"
newerr="$(mktemp /tmp/squelch.err.XXXXXX)"

eval "${@}" >"$newout" 2>"$newerr"
retval=$?

if [ $retval != 0 ]; then
  if [ $retval -eq $MAGICAL_FEISTY_MEOW_OKAY_RETURN_VALUE ]; then
    # special standard here means that we will still show the output, but there was no actual error.
    cat "$newout"
    # reset to a non-error for our special case.
    retval=0
  else
    # there was an actual error during the execution of the command.
    cat "$newout"
    cat "$newerr" >&2
    echo "An error was returned during execution of: ${@}" >&2
  fi
fi

# clean up.
rm "$newout" "$newerr"

# pass along the error code we saw, whether success or failure, so that this command has same
# exit result as the original would have.
exit $retval

