#!/bin/bash

# Author: Chris Koeritz

# This is the full orchestrator for bringing up a web site using our site
# management scripts.  So far, the scripts rely on at least php.  The support
# is much more powerful if the site is based on cakephp and site avenger.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../.." && \pwd )"

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

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

if (( $EUID != 0 )); then
  echo "This script must be run as root or sudo."
  exit 1
fi

if [ -z "$app_dirname" ]; then
  print_instructions
fi

source "$WORKDIR/shared_site_mgr.sh"

if [ "$app_dirname" == "-help" -o "$app_dirname" == "--help" ]; then
  print_instructions
fi

sep

check_application_dir "$APPLICATION_DIR"

add_domain "$DOMAIN_NAME"
test_or_die "Setting up domain: $DOMAIN_NAME"

add_apache_site "$APPLICATION_NAME" "$DOMAIN_NAME"
test_or_die "Setting up apache site for: $APPLICATION_NAME"

powerup "$APPLICATION_NAME" "$REPO_NAME" "$THEME_NAME"

sep

echo "
Finished standing up the full domain and site in:
${app_dirname}"

