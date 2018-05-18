#!/bin/bash

# creates a new apache website for a specified domain.

# auto-find the scripts, since we might want to run this as sudo.
export THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"

# some convenient defaults for our current usage.

if [ -z "$BASE_APPLICATION_PATH" ]; then
  BASE_APPLICATION_PATH="$HOME/apps"
fi
if [ -z "$STORAGE_SUFFIX" ]; then
  STORAGE_SUFFIX="/public"
fi

# main body of script.

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

appname="$1"; shift
site="$1"; shift
site_path="$1"; shift

if [ -z "$appname" -o -z "$site" ]; then
#hmmm: move to a print_instructions function.
  echo "
$(basename $0): {app name} {dns name} [site path]

This script needs to know (1) the application name for the new site and
(2) the DNS name for the apache virtual host.  The appname should be an
appropriate name for a file-system compatible folder name.  There is an
optional third parameter (3) the path for site storage.  If the site path
is not provided, we'll use this path:
  $BASE_APPLICATION_PATH/{app name}$STORAGE_SUFFIX"
  exit 1
fi

maybe_create_site_storage "$appname"
write_apache_config "$appname" "$site" "$site_path"
enable_site "$site"
restart_apache

