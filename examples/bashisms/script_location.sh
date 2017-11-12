
# find out the location where this script is running from.  this will not
# work properly in a bash script that is included via 'source' or '.'.
# the first letter of each command is escaped to eliminate the danger of
# personal aliases or functions disrupting the results.
ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"

# another slightly tighter version:
export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

