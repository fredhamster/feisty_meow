#!/bin/bash

# runs the less command but with some additional options.
#
# we always pass the raw flag (-r) so our special formatting is not lost.
# this may not be the best choice for some text terminals.
#
# there is also a "code" option added (-c) that formats the incoming data as
# if it were source code before showing the output.  this works great on files
# that are actually recognized as source code, and not so great on other
# things like simple lists or error output.
function lesser()
{
  code_view_flag="$1"
  if [ "$code_view_flag" == "-c" ]; then
    # saw the 'code' flag, which means show the file with source highlighting.
    shift
#echo saw the code flag

  else
    # drop the value and emptiness will mean don't show code.
    unset code_view_flag
  fi

  if [ ! -z "$code_view_flag" ]; then
    # establish a pipe for less to see our beloved syntax highlighting.
    export LESSOPEN="| source-highlight -f esc -o STDOUT -i %s"
  fi


  # run the source highlighter first if needed.
  /bin/less -r "${@}" 

#hmmm, will we need to iterate on the input array to get this right?

#hmmm: do we need to protect above from space hosings?
#  done, maybe.  if this still works for multiple parms...

}

# now that we have our function, just use it on all the input parameters.
# this is done to avoid adding yet another function to the core.
lesser "${@}" 


