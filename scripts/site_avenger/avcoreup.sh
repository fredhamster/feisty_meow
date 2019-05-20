#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# updates just the site avenger core portion of an app.

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

############################

# main body of script.

# check for parameters.
app_dirname="$1"; shift

source "$THISDIR/shared_site_mgr.sh"

sep

check_apps_root "$app_dirname"

# find proper webroot where the site will be initialized.
if [ -z "$app_dirname" ]; then
  # no dir was passed, so guess it.
  find_app_folder "$BASE_APPLICATION_PATH"
else
  test_app_folder "$BASE_APPLICATION_PATH" "$app_dirname"
fi
exit_on_error "finding and testing app folder"

# where we expect to find our checkout folder underneath.
full_app_dir="$BASE_APPLICATION_PATH/$app_dirname"

# simplistic approach here; just go to the folder and pull the changes.

pushd "$full_app_dir" &>/dev/null
exit_on_error "Changing to app path '$full_app_dir'"

dir="$CHECKOUT_DIR_NAME/vendor/siteavenger/avcore"
if [ ! -d $dir ]; then
  echo "The application doesn't seem to use avcore: $full_app_dir"
else
  pushd "$dir" &>/dev/null

  git pull
  exit_on_error "Pulling git repo for avcore under '$full_app_dir'"

  echo "Finished updating the avcore portion of site in ${app_dirname}."

  popd &>/dev/null
fi

popd &>/dev/null

sep

