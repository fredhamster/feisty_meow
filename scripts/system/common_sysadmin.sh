#!/bin/bash

# this is a library of functions shared by scripts in the system folder.
#
# Author: Chris Koeritz

# removes a full domain from the DNS.
function remove_domain_file()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"
  if [ -f "$domain_file" ]; then
    \rm -f "$domain_file"
    test_or_die "removing domain file: $domain_file"
  fi
}

# creates a totally new domain config file for DNS.
function write_new_domain_file()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"

  echo "adding a totally new domain called $domain_name"
  echo "using the config file: $domain_file"

  if [ -f $domain_file ]; then
    echo
    echo "The domain configuration file already exists at:"
    echo "  $domain_file"
    echo "Since we don't want to tear that down if it has specialized configuration"
    echo "data in it, we will just leave it in place and consider our job done."
    echo
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

  # our personalized configuration approach wants the real owner to own the file.
  chown "$(logname):$(logname)" $domain_file
  test_or_die "setting ownership on: $domain_file"
}

# takes a zone back out of the local conf file for bind
function remove_zone_for_domain()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"

  \cp -f "$domain_file" "$domain_file.bkup-${RANDOM}" 
  test_or_die "backing up domain file: $domain_file"

  # temp file to write to before we move file into place in bind.
  local new_version="/tmp/$domain_file.bkup-${RANDOM}" 
  \rm -f "$new_version"
  test_or_die "cleaning out new version of domain file from : $new_version"

  local line
  local skip_count=0
  while read line; do
    # don't bother looking at the lines if we're already in skip mode.
    if [[ $skip_count == 0 ]]; then
      # find the zone for the domain.
      if [[ ! "$line" =~ *"zone \"${domain_name}\""* ]]; then
        echo "$line" >> "$new_version"
      else
        # start skipping.  we will delete this line and the next 6 lines.
        ((skip_count++))
echo first skip count is now $skip_count
      fi
    else
      # we're already skipping.  let's keep going until we hit the limit.
      ((skip_count++))
      if [[ $skip_count >= 6 ]]; then
        echo "Done skipping, and back to writing output file."
        skip_count=0
      fi
    fi
  done < "$domain_file"

#put the file back into place.
echo file we created looks like this:
filedump "$new_version"

echo bailing
exit 1

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

  # keep ownership for the real user.
  chown "$(logname):$(logname)" /etc/bind/named.conf.local
  test_or_die "setting ownership on: /etc/bind/named.conf.local"

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

  # keep ownership for real user.
  chown "$(logname):$(logname)" "/etc/bind/${containing_domain}.conf"
  test_or_die "setting ownership on: /etc/bind/${containing_domain}.conf"
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

