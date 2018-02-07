#!/bin/bash

# a set of useful functions for managing operations with passwords.
# a set of very simple operations, but the file needs to be protected from
# undesirable access.  a good way to do that is to make the file owned by
# root, and for it to have permssions of "600" (full access by owner only),
# and to only try to read the password file when in sudo mode.  the first
# two requirements are done automatically by the store_password function.

# load_password:
# provides a way to read a password out of a file.  the filename is the first
# paramater and the variable to fill with the password is the second.
function load_password()
{
  local passfile="$1"; shift
  local varname="$1"; shift
  if [ -z "$passfile" ]; then
    echo 'The load_password function needs a filename to read the password from.'
    return 1
  fi
  if [ ! -f "$passfile" ]; then
    # no file, which is not an error necessarily, but return a blank password
    # in any case.
    return 0
  fi
  local passwd
  read passwd < "$passfile"

  # return the password in the variable they provided.
  eval $varname="$passwd"
}

# stores a password into a password file.  the password file should be the
# first parameter and the password should be the second.
# this makes sure that only root can read the file.
function store_password()
{
  local passfile="$1"; shift
  local passwd="$1"; shift
  if [ -z "$passfile" -o -z "$passwd" ]; then
    echo '
The store_password function needs (1) the file to store the password into,
and (2) the password that should be stored.
'
    return 1
  fi

  echo "$passwd" > "$passfile"
  test_or_die "writing password into the file $passfile"

  chown root:root "$passfile"
  test_or_die "chowning the password file to root ownership for: $passfile"

  chmod 600 "$passfile"
  test_or_die "restricting permissions on password file for: $passfile"
}

# reads a password from the console, without echoing the letters when they
# are typed.  the prompt to show the user is required as the first parameter,
# and the variable to fill with the result is the second parameter.
function read_password()
{
  local prompt="$1"; shift
  local varname="$1"; shift
#hmmm: complain if not enough parms.
  echo -n "$prompt "
  # turn off echo but remember former setting.
  stty_orig=`stty -g`
  stty -echo
  local the_passwd
  read the_passwd
  # turn echo back on.
  stty $stty_orig
  # return the password in the variable they provided.
  eval $varname="$the_passwd"
}


