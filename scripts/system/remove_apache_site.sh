#!/usr/bin/env bash

# uninstalls the apache website for a specified domain.

# auto-find the scripts, since we might want to run this as sudo.
export THISDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"

# some convenient defaults for our current usage.

if [ -z "$BASE_APPLICATION_PATH" ]; then
  BASE_APPLICATION_PATH="$FEISTY_MEOW_REPOS_SCAN"
#hmmm: take just first item!
fi
if [ -z "$STORAGE_SUFFIX" ]; then
  STORAGE_SUFFIX="/public"
fi

# main body of script.

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

site="$1"; shift

if [ -z "$site" ]; then
#hmmm: move to a print_instructions function.
  echo "
$(basename $0): {dns name} 

This script needs to know (1) the DNS name for the apache virtual host.
The script will uninstall that site's configuration files for apache2.
"
  exit 1
fi

disable_site "$site"
remove_apache_config "$site"
restart_apache

