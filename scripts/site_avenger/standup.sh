#!/bin/bash

# Author: Chris Koeritz

# This is the full orchestrator for bringing up a web site using our site
# management scripts.  So far, the scripts rely on at least php.  The support
# is much more powerful if the site is based on cakephp and site avenger.

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"

############################

function print_instructions()
{
  echo
  echo "$(basename $0 .sh) {app name}"
  echo
  echo "
$(basename $0 .sh) will completely set up a web site, including a domain
name and an apache configuration file.  The site will be acquired from a
git repository and configured.  At the end of this script, the result should 
be an almost working website; you may need to fix the site configuration,
create databases and so forth.

This script must be run as sudo or root; it makes changes to system files.
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

if [ "$app_dirname" == "-help" -o "$app_dirname" == "--help" ]; then
  print_instructions
elif [ -z "$app_dirname" ]; then
  print_instructions
fi

# force the sudo at the start of the script, rather than waiting halfway
# through to ask for access.
sudo bash -c 'echo sudo permissions acquired.'

source "$WORKDIR/shared_site_mgr.sh"

sep

check_application_dir "$BASE_APPLICATION_PATH"

# find proper webroot where the site will be initialized.
if [ -z "$app_dirname" ]; then
  # no dir was passed, so guess it.
  find_app_folder "$BASE_APPLICATION_PATH"
else
  test_app_folder "$BASE_APPLICATION_PATH" "$app_dirname"
fi

#echo "!! domain being added is: $DOMAIN_NAME"

sudo bash "$FEISTY_MEOW_SCRIPTS/system/add_domain.sh" "$DOMAIN_NAME"
test_or_die "Setting up domain: $DOMAIN_NAME"

sep

# add the main website as specified by the domain name they gave us.
sudo bash "$FEISTY_MEOW_SCRIPTS/system/add_apache_site.sh" "$APPLICATION_NAME" "$DOMAIN_NAME"
test_or_die "Setting up apache site for: $APPLICATION_NAME"

# make the shadow site also, which always ends in cakelampvm.com.
shadow_domain="${APPLICATION_NAME}.cakelampvm.com"
if [ "$shadow_domain" != "$DOMAIN_NAME" ]; then
  sudo bash "$FEISTY_MEOW_SCRIPTS/system/add_apache_site.sh" "$APPLICATION_NAME" "$shadow_domain"
  test_or_die "Setting up shadow apache site on '$shadow_domain'"
fi

sep

#echo about to do powerup with: app="$APPLICATION_NAME" repo="$REPO_NAME" theme="$THEME_NAME"
#echo default repo is "$DEFAULT_REPOSITORY_ROOT" 

powerup "$APPLICATION_NAME" "$REPO_NAME" "$THEME_NAME"
# pass the real user name who should own the files.
# "$(logname)"

sep

echo "
Finished standing up the full domain and site for: ${app_dirname}
The domain name is: $DOMAIN_NAME
"

