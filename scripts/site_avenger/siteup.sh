#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# updates a site avenger app.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

############################

# main body of script.

# check for parameters.
app_dirname="$1"; shift
repo_name="$1"; shift

source "$WORKDIR/shared_site_mgr.sh"

sep

check_apps_root "$BASE_APPLICATION_PATH"

# find proper webroot where the site will be initialized.
if [ -z "$app_dirname" ]; then
  # no dir was passed, so guess it.
  find_app_folder "$BASE_APPLICATION_PATH"
else
  test_app_folder "$BASE_APPLICATION_PATH" "$app_dirname"
fi
test_or_die "finding and testing app folder"

# where we expect to find our checkout folder underneath.
full_app_dir="$BASE_APPLICATION_PATH/$app_dirname"

# use our default values for the repository and theme if they're not provided.
if [ -z "$repo_name" ]; then
  repo_name="$app_dirname"
fi

echo "Repository: $repo_name"
sep

# this should set the site_store_path variable if everything goes well.
update_repo "$full_app_dir" "$CHECKOUT_DIR_NAME" "$DEFAULT_REPOSITORY_ROOT" "$repo_name"
test_or_die "Updating the repository storage directory"

####

sep

echo "Finished updating the site in ${app_dirname}."

