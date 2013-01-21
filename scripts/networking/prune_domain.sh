#!/bin/bash

# takes the domain name off of the hostname provided and outputs just the
# short name for the host.  the host is the first parameter, and the domain
# name is the second, but both are optional.  if missing, this script uses
# the local machine's information to print a hostname stripped of domain.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

#ha, should look back and see what other cool stuff hecho used to do.
function hecho()
{
  echo $* 1>&2
}

export host="$1"; shift
if [ -z "$host" ]; then
  host="$(hostname)"
fi
export DOMAIN_NAME="$1"; shift
if [ -z "$DOMAIN_NAME" ]; then
  # set the domain name based on what's found in the resolv.conf file, where
  # dns info is often found.
  DOMAIN_NAME=$(grep -i search </etc/resolv.conf | tail -1 | sed -n -e 's/domain.\(.*\)$/\1/p')
  if [ -z "$DOMAIN_NAME" ]; then
    # second try, searching out the search domain as a suitable replacement
    # for the actual domain being specified.
    DOMAIN_NAME=$(grep -i search </etc/resolv.conf | tail -1 | sed -n -e 's/search.\(.*\)$/\1/p')
  fi
  # oh well, we don't know what the heck the domain is.
  if [ -z "$DOMAIN_NAME" ]; then
    hecho "The domain name cannot be intuited from the system and was not provided"
    hecho "on the command line, so we have to give up."
#hmmm: below starts to look reusable.
    hecho "   host is '$host', domain is'$DOMAIN_NAME'."
    exit 1
  fi
fi

#hecho domain is $DOMAIN_NAME
#hecho name before was $host

domlen=${#DOMAIN_NAME}
namelen=${#host}

if [ $namelen -le $(($domlen+1)) ]; then
  hecho "The hostname is too short to remove the domain name."
  hecho "   host is '$host', domain is'$DOMAIN_NAME'."
  exit 1
fi

#hecho domain name length is $domlen

# we go back one more character than needed for the domain name, so we can consume the dot.
chopname=${host:0:(($namelen - $domlen - 1))}

#hecho "chopped name is '$chopname'"

echo $chopname

