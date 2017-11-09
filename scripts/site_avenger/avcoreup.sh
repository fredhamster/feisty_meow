#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# updates just the site avenger core portion of an app.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
source "$WORKDIR/shared_site_mgr.sh"

# get our defaults.
source "$WORKDIR/site_avenger.config"

############################

# main body of script.

# check for parameters.
app_dirname="$1"; shift

sep

check_application_dir "$APPLICATION_DIR"

# find proper webroot where the site will be initialized.
if [ -z "$app_dirname" ]; then
  # no dir was passed, so guess it.
  find_app_folder "$APPLICATION_DIR"
else
  test_app_folder "$APPLICATION_DIR" "$app_dirname"
fi

# where we expect to find our checkout folder underneath.
full_app_dir="$APPLICATION_DIR/$app_dirname"

# simplistic approach here; just go to the folder and pull the changes.

pushd "$full_app_dir" &>/dev/null
test_or_die "Changing to app path '$full_app_dir'"

dir="avenger5/vendor/siteavenger/avcore"
if [ ! -d $dir ]; then
  echo "The application doesn't seem to use avcore: $full_app_dir"
else
  pushd "$dir" &>/dev/null

  git pull
  test_or_die "Pulling git repo for avcore under '$full_app_dir'"

  echo "Finished updating the avcore portion of site in ${app_dirname}."

  popd &>/dev/null
fi

popd &>/dev/null

sep

