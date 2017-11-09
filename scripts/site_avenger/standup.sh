#!/bin/bash

# Author: Chris Koeritz

# This is the full orchestrator for bringing up a web site using our site
# management scripts.  So far, the scripts rely on at least php.  The support
# is much more powerful if the site is based on cakephp and site avenger.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

source "$WORKDIR/shared_site_mgr.sh"

############################

function print_instructions()
{
  echo
  echo "$(basename $0 .sh) {app name}"
  echo
  echo "
app name: The app name parameter is mandatory.  The configuration file for
this script will be derived from the app name (e.g. if the app name is MyApp,
then the config file will be 'MyApp.config').  The config files are by
convention stored in the 'config' directory.  The configuration file can be
overridden by setting the SITE_MANAGEMENT_CONFIG_FILE environment variable."
  exit 0
}

############################

# main body of script.

# check for parameters.
app_dirname="$1"; shift
repo_name="$1"; shift
theme_name="$1"; shift

if [ "$app_dirname" == "-help" -o "$app_dirname" == "--help" ]; then
  print_instructions
fi

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
if [ -z "$theme_name" ]; then
  theme_name="$(capitalize_first_char ${app_dirname})"
fi

echo "Repository: $repo_name"
echo "Theme name: $theme_name"
sep

# this should set the site_store_path variable if everything goes well.
update_repo "$full_app_dir" "$CHECKOUT_DIR_NAME" "$DEFAULT_REPOSITORY_ROOT" "$repo_name"
test_or_die "Updating the repository storage directory"

# update the site to load dependencies.
sep
composer_repuff "$site_store_path"
test_or_die "Installing site dependencies with composer"

# set up the symbolic links needed to achieve siteliness.
sep

create_site_links "$site_store_path" "$theme_name"

sep

echo "Finished powering up the site in '${app_dirname}'."

