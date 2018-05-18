#!/bin/bash

# this set of functions serve the main purpose of adding new domains or
# subdomains to the bind9 DNS server on the current host.  it is currently
# highly specific to running a bunch of domains on a linux VM, where the VM
# has one IP address.  note that the bind 'named' must already be configured.
# also, it is assumed that, if a subdomain is being added, then the containing
# domain has already been configured and is configured in a file similar to
# "blah.com.conf" in /etc/bind.
#
# Author: Chris Koeritz

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"

# some defaults that are convenient for current purposes.
# existing values will be respected over our defaults.

if [ -z "$IP_ADDRESS" ]; then
  # in our scheme, the single IP address that all our domains map to.
  IP_ADDRESS="10.28.42.20"
fi
if [ -z "$SERVER_ADMIN" ]; then
  # the email address (where first dot is replaced by @) for the administrator of the domain.
  SERVER_ADMIN="developer.cakelampvm.com"
fi
if [ -z "$MAIN_NAME_SERVER" ]; then
  # the name of the name server for the new domains (should already be configured).
  MAIN_NAME_SERVER="ns.cakelampvm.com"
fi
if [ -z "$MAIL_SERVER" ]; then
  # the name of the mail server for a new domain (should already be configured).
  MAIL_SERVER="mail.cakelampvm.com"
fi
if [ -z "$DISTRO" ]; then
  # the distribution name to be listed in info for the new domain or subdomain.
  DISTRO="ubuntu"
fi

# main body of script.

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

new_domain="$1"; shift

if [ -z "$new_domain" ]; then
  echo "This script needs a domain name to add to DNS." 
  exit 1
fi

# if domain name has three or more components, then add a subdomain.
# otherwise, add a full new domain.
if [[ $new_domain == *"."*"."* ]]; then
  # add a subdomain to the containing domain.
  add_new_subdomain "$new_domain"
  restart_bind
else
  # create a totally new domain in DNS.
  write_new_domain_file "$new_domain"
  add_zone_for_new_domain "$new_domain"
  restart_bind
fi


