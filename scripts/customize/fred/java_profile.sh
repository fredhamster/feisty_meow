#!/usr/bin/env bash

# Author: Chris Koeritz

# this script tries to intuit where java is installed on this machine.

############################

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

# this reports when we have totally failed to figure out where a folder
# is actually located on the machine.
function intuition_failure()
{
  missing="$1"; shift
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "Could not intuit '$missing' variable."
  fi
  # remove the variable because its value is busted.
  unset $missing
}

############################

# start guessing some settings...

# whatever we figure out, we want to export the java home variable.
export JAVA_HOME

# this bin portion works for most javas...
export JAVA_BIN_PIECE=bin

# try using java itself to locate the JAVA_HOME if we can.
if [ ! -d "$JAVA_HOME" ]; then
  JAVA_HOME=$(java -XshowSettings:properties -version 2>&1 | grep -i java.home | sed -e 's/.*java.home = \(.*\)$/\1/')
fi

# if that didn't work, then we try a series of random bizarro places where
# we have seen java live before.

#hmmm: below list is way out of date.  we really hope the first attempt above works.

if [ ! -d "$JAVA_HOME" ]; then
  # try a recent version.
  export JAVA_HOME=/usr/lib/jvm/java-8-oracle
fi
if [ ! -d "$JAVA_HOME" ]; then
  # or an older version.
  export JAVA_HOME=/usr/lib/jvm/java-7-oracle
fi
if [ ! -d "$JAVA_HOME" ]; then
  JAVA_HOME="$(ls -d c:/tools/*jdk* 2>/dev/null)"
fi
if [ ! -d "$JAVA_HOME" ]; then
  JAVA_HOME="$(ls -d "c:/Program Files"/*jdk* 2>/dev/null)"
fi
if [ ! -d "$JAVA_HOME" ]; then
  JAVA_HOME="$(ls -d "c:/Program Files (x86)"/*jdk* 2>/dev/null)"
fi
if [ ! -d "$JAVA_HOME" ]; then
  if [ ! -z "$(grep -i 'd:' /proc/mounts 2>/dev/null)" ]; then
    # try using a windows version.
    JAVA_HOME="$(ls -d d:/tools/*jdk* 2>/dev/null)"
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
  unset JAVA_HOME
  unset JAVA_BIN_PIECE
  if [ -z "$(whichable java)" ]; then
    intuition_failure JAVA_HOME
  fi
fi

############################

# intuit where we have our local eclipse.
if [ ! -d "$ECLIPSE_DIR" ]; then
  export ECLIPSE_DIR=/usr/local/eclipse
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR=$HOME/eclipse
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR=/usr/local/fred/eclipse
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  ECLIPSE_DIR="c:/tools/eclipse"
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  if [ ! -z "$(grep -i 'd:' /proc/mounts 2>/dev/null)" ]; then
    ECLIPSE_DIR="d:/tools/eclipse"
  fi
fi
if [ ! -d "$ECLIPSE_DIR" ]; then
  if [ ! -z "$(grep -i 'e:' /proc/mounts 2>/dev/null)" ]; then
    ECLIPSE_DIR="e:/tools/eclipse"
  fi
fi
# final option is to whine.
if [ ! -d "$ECLIPSE_DIR" ]; then
  unset ECLIPSE_DIR
else
  if [ ! -z "$(uname -a | grep -i cygwin)" ]; then
    # fix the path for cygwin's bizarre requirement of /cygdrive/X.
    ECLIPSE_DIR=$(echo $ECLIPSE_DIR | sed -e 's/^\(.\):/\/cygdrive\/\1/')
  fi
fi
if [ -z "$ECLIPSE_DIR" -a -z "$(whichable eclipse)" ]; then
  intuition_failure ECLIPSE_DIR
fi

############################

# use the variables we just set in our path, and try to make them override
# any other paths to different versions.

if [ ! -z "$JAVA_HOME" ]; then
  j="$JAVA_HOME"
  if [ ! -z "$(uname -a | grep -i cygwin)" ]; then
    j=$(echo $j | sed -e 's/^\(.\):/\/cygdrive\/\1/')
  fi
  export PATH=$j/$JAVA_BIN_PIECE:$PATH
fi
if [ ! -z "$ECLIPSE_DIR" ]; then
  e="$ECLIPSE_DIR"
  if [ ! -z "$(uname -a | grep -i cygwin)" ]; then
    e=$(echo $e | sed -e 's/^\(.\):/\/cygdrive\/\1/')
  fi
  export PATH=$e:$PATH
fi
# add in our personal bin path.
if [ -d "/usr/local/fred/bin" ]; then
  export PATH=/usr/local/fred/bin:$PATH
fi

############################

#echo "java_profile: JAVA_HOME='$JAVA_HOME' ECLIPSE_DIR='$ECLIPSE_DIR'"

