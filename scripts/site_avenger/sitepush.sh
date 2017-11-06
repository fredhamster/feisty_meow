#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# checks the chosen site into the online git repository.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
source "$WORKDIR/shared_site_mgr.sh"

# get our defaults.
source "$WORKDIR/site_avenger.config"

############################

# main body of script.

# check for parameters.
app_dirname="$1"; shift
repo_name="$1"; shift

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

# use our default values for the repository and theme if they're not provided.
if [ -z "$repo_name" ]; then
  repo_name="$app_dirname"
fi

echo "Repository: $repo_name"
sep

# this should set the site_store_path variable if everything goes well.
update_repo "$full_app_dir" "$CHECKOUT_DIR_NAME" "$DEFAULT_REPOSITORY_ROOT" "$repo_name"
test_or_fail "Updating the repository storage directory"

sep

update_composer_repository "$site_store_path" 

sep

# now finally do the real check-in for our site.

pushd "$site_store_path" &>/dev/null
rcheckin

sep

echo "Finished checking in the site at ${app_dirname}."

