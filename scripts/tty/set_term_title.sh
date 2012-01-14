#!/bin/bash

# terminal labeller: sets the current terminal's title to the arguments
# specified on the command line, or if those are blank, set the title to
# the hostname.

title="$*"
if [ -z "${title}" ]; then
  title="$(hostname)"
fi
#echo title will be $title

echo -n -e "\033]0;${title}\007"

