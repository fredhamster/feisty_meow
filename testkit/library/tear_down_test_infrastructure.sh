#!/usr/bin/env bash

# Tears down the demo users previously set up for multi-user testing.
#
# Author: Chris Koeritz

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd "$WORKDIR"

if [ -z "$TESTKIT_SENTINEL" ]; then echo Please run prepare_tools.sh before testing.; exit 3; fi
source "$TESTKIT_ROOT/library/establish_environment.sh"

progname="$(basename "$0")"

if [ $# -lt 1 ]; then
  echo "$progname: This script needs a single parameter, which is the container"
  echo "path to use for the authentication (e.g. $STS_LOC)"
  exit 3
fi

ADMIN_CONTAINER=$1; shift
echo "container location is $ADMIN_CONTAINER "

# we test for ten users currently.
user_count=10

# login the right power user that can delete other user entries.
testLoginAsAdmin()
{
  if [ -z "$NON_INTERACTIVE" ]; then
    login_a_user admin
  fi
}

# now that we're logged in appropriately, delete our corresponding set of users.
testDeleteUsers()
{
  local x
  for (( x=0; x < ${#MULTI_USER_LIST[*]}; x++ )); do
    username="${MULTI_USER_LIST[$x]}"
    echo "Whacking user '$username'..."
    passwd="${MULTI_PASSWORD_LIST[$x]}"
    # now do the heavy lifting to get that user set up.
    silent_grid script "local:'$TESTKIT_ROOT/library/delete_one_user.xml'" "$CONTAINERPATH" "$(basename $username)" "$username" "$SUBMIT_GROUP"
    assertEquals "Should delete user '$username' successfully" 0 $?
  done
}

# make sure we don't leave them logged in as an administrator.
testLogoutAgain()
{
  if [ -z "$NON_INTERACTIVE" ]; then
    silent_grid logout --all
    assertEquals "Final logout of the grid" 0 $?
  fi
}

testLoginNormalUser()
{
  if [ -z "$NON_INTERACTIVE" ]; then
    login_a_user normal
  fi
}

# load and run shUnit2
source "$SHUNIT_DIR/shunit2"

