#!/usr/bin/env bash

# this performs the inverse operation of standup, by relying on the
# remove_domain and remove_apache_site scripts.
#
# Author: Chris Koeritz

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"

############################

function print_instructions()
{
  echo
  echo "$(basename $0 .sh) {app name}"
  echo
  echo "
$(basename $0 .sh) will drop a web site out of apache server and out of the
DNS server, as if it never existed.  The site storage is left untouched; we
don't know what valuable assets lurk there.
This script must be run as sudo or root; it makes changes to system files.
"
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

sep

sudo bash "$FEISTY_MEOW_SCRIPTS/system/remove_apache_site.sh" "$DOMAIN_NAME"
exit_on_error "dropping apache site for: $DOMAIN_NAME"

# drop the shadow site too.
shadow_domain="${APPLICATION_NAME}.cakelampvm.com"
if [ "$shadow_domain" != "$DOMAIN_NAME" ]; then
  sudo bash "$FEISTY_MEOW_SCRIPTS/system/remove_apache_site.sh" "$shadow_domain"
  exit_on_error "dropping shadow apache site on '$shadow_domain'"
fi

sep

#echo "!! domain being removed is: $DOMAIN_NAME"

sudo bash "$FEISTY_MEOW_SCRIPTS/system/remove_domain.sh" "$DOMAIN_NAME"
exit_on_error "dropping domain: $DOMAIN_NAME"

sep

echo "
Finished tearing down the domain name and apache site for:
  $DOMAIN_NAME
"

