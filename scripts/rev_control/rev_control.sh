#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# one unpleasantry to take care of first; cygwin barfs aggressively if the TMP directory
# is a DOS path, but we need it to be a DOS path for our XSEDE testing, so that blows.
# to get past this, TMP gets changed below to a hopefully generic and safe place.

export TMP=/tmp/rev_control_$USER
if [ ! -d "$TMP" ]; then
  mkdir $TMP
fi
if [ ! -d "$TMP" ]; then
  echo "Could not create the temporary directory TMP in: $TMP"
  echo "This script will not work properly without an existing TMP directory."
fi

this_host=
# gets the machine's hostname and stores it in the variable "this_host".
function get_our_hostname()
{
  if [ "$OS" == "Windows_NT" ]; then
    this_host=$(hostname)
  elif [ ! -z "$(echo $MACHTYPE | grep apple)" ]; then
    this_host=$(hostname)
  elif [ ! -z "$(echo $MACHTYPE | grep suse)" ]; then
    this_host=$(hostname --long)
  else
    this_host=$(hostname)
  fi
  #echo "hostname is $this_host"
}

# this function sets a variable called "home_system" to "true" if the
# machine is considered one of fred's home machines.  if you are not
# fred, you may want to change the machine choices.
export home_system=
function is_home_system()
{
  # load up the name of the host.
  get_our_hostname
  # reset the variable that we'll be setting.
  home_system=
  if [[ $this_host == *.gruntose.blurgh ]]; then
    home_system=true
#temp code
elif [[ $this_host == buildy ]]; then
home_system=true
elif [[ $this_host == simmy ]]; then
home_system=true
#temp code
  fi
}

# we only want to totally personalize this script if the user is right.
function check_user()
{
  if [ "$USER" == "fred" ]; then
    export SVNUSER=fred_t_hamster@
    export EXTRA_PROTOCOL=+ssh
  else
    export SVNUSER=
    export EXTRA_PROTOCOL=
  fi
}

# calculates the right modifier for hostnames / repositories.
modifier=
function compute_modifier()
{
  modifier=
  directory="$1"; shift
  in_or_out="$1"; shift
  check_user
  # some project specific overrides.
  if [[ "$directory" == hoople* ]]; then
    modifier="svn${EXTRA_PROTOCOL}://${SVNUSER}svn.code.sf.net/p/hoople2/svn/"
  fi
  if [[ "$directory" == yeti* ]]; then
    modifier="svn${EXTRA_PROTOCOL}://${SVNUSER}svn.code.sf.net/p/yeti/svn/"
  fi
  # see if we're on one of fred's home machines.
  is_home_system
  # special override to pick local servers when at home.
  if [ "$home_system" == "true" ]; then
    if [ "$in_or_out" == "out" ]; then
      # need the right home machine for modifier when checking out.
      modifier="svn://shaggy/"
    else 
      # no modifier for checkin.
      modifier=
    fi
  fi
}

