#!/bin/bash

# this set of functions serve the main purpose of adding new domains or subdomains to the bind9 DNS server on the current host.
# it is currently highly specific to running a bunch of domains on a linux VM, where the VM has one IP address.
# note that bind 'named' must already be configured.
# also, it is assumed that if a subdomain is being added, then the containing domain has already been configured and is 
# configured in a file similar to "blah.com.conf" in /etc/bind.
#
# Author: Chris Koeritz

# some defaults that are convenient for current purposes.
# hmmm: these would need to be parameterized somehow for this script to become really general.

# in our scheme, the single IP address that all our domains map to.
IP_ADDRESS="10.28.42.20"
# the email address (where first dot is replaced by @) for the administrator of the domain.
SERVER_ADMIN="fred.cakelampvm.com"
# the name of the name server for the new domains (should already be configured).
MAIN_NAME_SERVER="ns.cakelampvm.com"
# the name of the mail server for a new domain (should already be configured).
MAIL_SERVER="mail.cakelampvm.com"
# the distribution name to be listed in info for the new domain or subdomain.
DISTRO="ubuntu"

# creates a totally new domain config file for DNS.
function write_new_domain_file()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"

  echo "adding a totally new domain called $domain_name"
  echo "using the config file: $domain_file"

  if [ -f $domain_file ]; then
    echo "The domain configuration file already exists at:"
    echo "  $domain_file"
    echo "Since we don't want to tear that down if it has specialized configuration"
    echo "data in it, we will just leave it in place and consider our job done.
    exit 0
  fi

  echo "
\$TTL 1W
@	IN SOA	@	${SERVER_ADMIN}. (
		2017100801 ; serial
		2H ; refresh
		8M ; retry
		14D ; expiry
		6H ) ; minimum

	IN NS		${MAIN_NAME_SERVER}.
	IN MX	10	${MAIL_SERVER}.

${domain_name}.	IN A	${IP_ADDRESS}
	IN HINFO	\"linux server\" \"${DISTRO}\"
" >"$domain_file"
}

# hooks up a new config file into bind's list of zones.
function add_zone_for_new_domain()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"

  echo "adding a new domain configured by ${domain_file} into"
  echo "the named.conf.local configuration file."

  # append the reference to the new conf file in the zone list.
  echo "
zone \"${domain_name}\" in {
	file \"${domain_file}\";
	type master;
	allow-query { any; };
};

////////////////////////////////////////////////////////////////////////////

" >> /etc/bind/named.conf.local
}

# adds a new subdomain under a containing domain.
function add_new_subdomain()
{
  local new_domain="$1"; shift

  # split up the full domain name into subdomain portion and containing domain.
  local subdomain="${new_domain%.*.*}"
  local containing_domain="${new_domain#*.}"

  echo "adding a subdomain $subdomain to containing domain $containing_domain"

  local domain_file="/etc/bind/${containing_domain}.conf"
  # see if config file already exists; if not, complain.
  if [ ! -f "$domain_file" ]; then
    echo "The domain configuration file for $new_domain is missing."
    echo "It should already be present in: $domain_file"
    echo "Please add the containing domain before trying to add a subdomain."
    exit 1
  fi

  # see if subdomain already present in config.
  if [ $(grep -q "$new_domain" "$domain_file") ]; then
    echo "The subdomain $subdomain already seems to exist in the domain"
    echo "configuration file: $domain_file"
    echo "Please edit the config file to remove the subdomain before trying"
    echo "to re-add the subdomain."
    exit 1
  fi

  # append the new subdomain into the config file.
  echo "
${subdomain}.${containing_domain}.    IN A    ${IP_ADDRESS}
        IN HINFO \"linux server\" \"${DISTRO}\"
" >> /etc/bind/${containing_domain}.conf

}

function restart_bind()
{
  echo restarting DNS server.
  service bind9 restart
  if [ $? -ne 0 ]; then
    echo "The bind service did not restart properly.  Please check the error logs."
    exit 1
  fi
  echo DNS server restarted.
}

# main body of script.

if (( $EUID != 0 )); then
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


