#!/bin/bash

# Author: Chris Koeritz

# this script tries to intuit where java is installed on this machine.

############################

function intuition_failure()
{
  missing="$1"; shift
  echo "We cannot intuit your $missing variable for this host."
  # remove the variable because its value is busted.
  unset $missing
}

############################

# set some fairly liberal limits for ant.
export ANT_OPTS="-Xms512m -Xmx768m -XX:MaxPermSize=768m"

############################

# start guessing some settings...

# this bin portion works for most javas...
export JAVA_BIN_PIECE=bin

if [ ! -d "$JAVA_HOME" ]; then
  # first try a recent linux version.
  export JAVA_HOME=/usr/lib/jvm/java-6-sun/jre
fi
if [ ! -d "$JAVA_HOME" ]; then
  # try an even more recent version.
  export JAVA_HOME=/usr/lib/jvm/java-7-oracle/jre
fi
if [ ! -d "$JAVA_HOME" ]; then
  # try using a windows version.
#note: this logic is untested.
# probably will break due to space in path issues.
  declare -a any_there=$(find "/c/Program Files/java" -type d -iname "jdk" 2>/dev/null)
  if [ ${#any_there[*]} -gt 0 ]; then
    (( last = ${#any_there[@]} - 1 ))
    JAVA_HOME="${any_there[$last]}"
  fi
  if [ ! -d "$JAVA_HOME" ]; then
    # if no jdk, try a jre.
    declare -a any_there=$(find "/c/Program Files/java" -type d -iname "jre" 2>/dev/null)
    if [ ${#any_there[*]} -gt 0 ]; then
      (( last = ${#any_there[@]} - 1 ))
      JAVA_HOME="${any_there[$last]}"
    fi
  fi
fi
# this should go last, since it changes the bin dir.
if [ ! -d "$JAVA_HOME" ]; then
  # if that didn't work, try the location for mac os x.
  JAVA_HOME=/Library/Java/Home
  JAVA_BIN_PIECE=Commands
fi
# last thing is to tell them we couldn't find it.
if [ ! -d "$JAVA_HOME" ]; then
  if [ ! -z "$(which java)" ]; then
    echo "JAVA_HOME unknown, but java is in path."
  else
    intuition_failure JAVA_HOME
    unset JAVA_BIN_PIECE
  fi
fi

############################

# intuit where we have our local eclipse.
if [ ! -d "$ECLIPSE_DIR" ]; then
  export ECLIPSE_DIR=/usr/local/eclipse_jee
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR=$HOME/eclipse
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR=$HOME/apps/eclipse
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
#uhhh, default on winders?
  ECLIPSE_DIR="/c/Program Files/eclipse"
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR="/c/tools/eclipse"
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR="/d/tools/eclipse"
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR="/e/tools/eclipse"
fi
# final option is to whine.
if [ ! -d "$ECLIPSE_DIR" ]; then
  if [ ! -z "$(which eclipse)" ]; then
    echo "ECLIPSE_DIR unknown, but eclipse is in path."
  else
    intuition_failure ECLIPSE_DIR;
  fi
fi

############################

# use the variables we just set in our path, and try to make them override
# any other paths to different versions.

if [ ! -z "$JAVA_HOME" ]; then
  export PATH=$JAVA_HOME/$JAVA_BIN_PIECE:$PATH
fi
if [ ! -z "$ECLIPSE_DIR" ]; then
  export PATH=$ECLIPSE_DIR:$PATH
fi

############################


