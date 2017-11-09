#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# This script "powers up" a cakephp site by running the database migrations,
# cleaning out the ORM cache, and fixing file permissions.
# Note that the mysql database must already exist and allow permissions to
# the configured username/password in config/app.php.
# This script is currently highly specific to site avenger.

# General Info:
#
# The naming scheme here is a little complex, but it's basically this...
# A git repository is expected to be provided, and we will get all the code
# for the web site from there.  The repository is expected to have a single
# application "name" and one or more "themes".  By convention, the name
# and the theme are often the same.
# For example, let's say our app name is "turtle" and our theme name is "box".
# The repo is checked out to a folder called "~/apps/turtle".
# This script will want to use "turtle" as the app name.
# It will have to be told the theme name, but will assume it's 'Turtle' to
# start with.  The concept of the theme comes from cakephp.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
source "$WORKDIR/shared_site_mgr.sh"

# get our defaults.
source "$WORKDIR/site_avenger.config"

############################

function print_instructions()
{
  echo
  echo "$(basename $0 .sh) [app dirname] [repository] [theme name]"
  echo
  echo "All parameters are optional, and intelligent guesses for them will be made."
  echo
  echo "app dirname: The folder where the app will be stored."
  echo "repository: The name of the git repository (short version, no URL)."
  echo "theme name: The name to use for the cakephp theme."
  echo
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

