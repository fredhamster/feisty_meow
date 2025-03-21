#!/usr/bin/env bash

# runs the less command but with some additional options.
#
# there is a "code" option (-c) that formats the incoming data as if it were source code before
# displaying the output.  this works great on files that are actually recognized as source code,
# and not so great on other things like simple lists or error output.
function lesser()
{
  local code_view_flag="$1"

  EXTRA_OPTIONS=

  if [ "$code_view_flag" == "-c" ]; then
    # saw the 'code' flag, which means show the file with source highlighting.
    shift

    # we always pass the raw flag (-r) when in code view mode so special formatting is not lost.
    # this may not work for some text terminals.
    EXTRA_OPTIONS=-r

  else
    # drop the value and emptiness will mean don't show code.
    unset code_view_flag
  fi

  if [ ! -z "$code_view_flag" ]; then
    # establish a pipe for less to see our beloved syntax highlighting.
    export LESSOPEN="| source-highlight -f esc -o STDOUT -i %s"
  fi

  # run the source highlighter first if needed.
  /usr/bin/env less $EXTRA_OPTIONS "${@}" 
}

##############

# now that we have our function, just use it on all the input parameters.
# this is done to avoid adding yet another function to the core.
lesser "${@}" 

