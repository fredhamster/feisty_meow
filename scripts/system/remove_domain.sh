#!/bin/bash

# performs the inverse function of add_domain by deconfiguring a domain
# in bind.  the domain needs to have been set up by add_domain, or this will
# not succeed.
#
# Author: Chris Koeritz

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"

# some defaults that are convenient for current purposes.
# existing values will be respected over our defaults.

# main body of script.

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

old_domain="$1"; shift

if [ -z "$old_domain" ]; then
  echo "This script needs a domain name to remove from DNS." 
  exit 1
fi

# if domain name has three or more components, then remove a subdomain.
# otherwise, remove a full domain.
if [[ $old_domain == *"."*"."* ]]; then
  # remove a subdomain from the containing domain.
  remove_subdomain "$old_domain"
  restart_bind
else
  # remove the full domain in DNS.
  remove_domain_file "$old_domain"
  remove_zone_for_domain "$old_domain"
  restart_bind
fi


