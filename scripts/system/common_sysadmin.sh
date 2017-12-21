#!/bin/bash

# this is a library of functions shared by scripts in the system folder.
#
# Author: Chris Koeritz

############################################################################

# bind9 methods...

# removes a full domain from the DNS.
function remove_domain_file()
{
  local domain_name="$1"; shift

  local domain_file="/etc/bind/${domain_name}.conf"
  if [ -f "$domain_file" ]; then
    # don't destroy, just shuffle.
    \mv -f "$domain_file" "/tmp/$(basename ${domain_file})-old-${RANDOM}"
    test_or_die "removing domain file: $domain_file"
  else
    echo "Did not see a domain file to remove: $domain_file"
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

  # eat the zone file definition.  this will botch up badly if more text was added
  # or the zone info shrank.
  create_chomped_copy_of_file "/etc/bind/named.conf.local" "zone.*${domain_name}" 6
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

# zaps a subdomain out of the containing domain file.
function remove_subdomain()
{
  local old_domain="$1"; shift

  # split up the full domain name into subdomain portion and containing domain.
  local subdomain="${old_domain%.*.*}"
  local containing_domain="${old_domain#*.}"

  echo "removing subdomain $subdomain from containing domain $containing_domain"

  local domain_file="/etc/bind/${containing_domain}.conf"
  # see if config file already exists; if not, complain.
  if [ ! -f "$domain_file" ]; then
    echo "The domain configuration file for $old_domain is missing."
    echo "It should already be present in: $domain_file"
    echo "We cannot remove a subdomain if the containing domain isn't there."
    exit 1
  fi

  # see if subdomain already present in config.
  if ! grep -q "$old_domain" "$domain_file"; then
    echo "The subdomain $subdomain is already missing from the domain"
    echo "configuration file: $domain_file"
    echo "Our work is apparently done for removing it."
    return 0
  fi

  create_chomped_copy_of_file "$domain_file" "${old_domain}" 2
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
  if grep -q "$new_domain" "$domain_file"; then
    echo "The subdomain $subdomain already seems to exist in the domain"
    echo "configuration file: $domain_file"
    echo "We are considering our work done; if you want to modify the subdomain,"
    echo "then please call remove_domain on it first."
    return 0
  fi

  # append the new subdomain into the config file.
  echo "${subdomain}.${containing_domain}.    IN A    ${IP_ADDRESS}
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
  echo DNS service restarted.
}

############################################################################

# samba server helper functions...

function restart_samba
{
  echo restarting samba server.
  service smbd restart
  if [ $? -ne 0 ]; then
    echo "The samba service did not restart properly.  Please check the error logs."
    exit 1
  fi
  service nmbd restart
  if [ $? -ne 0 ]; then
    echo "The samba name service (nmbd) did not restart properly.  This may not always be fatal, so we are ignoring it, but you may want to check the error logs."
  fi
  echo samba service restarted.
}

############################################################################

# apache2 methods...

# removes a config file for apache given the app name and site name.
function remove_apache_config()
{
  local sitename="$1"; shift

  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  if [ -f "$site_config" ]; then
    # don't destroy, just shuffle.
    \mv -f "$site_config" "/tmp/$(basename ${site_config})-old-${RANDOM}"
    test_or_die "removing site config: $site_config"
  else
    echo "Did not see a site config to remove: $site_config"
  fi
}

# this function writes out the new configuration file for the site.
function write_apache_config()
{
  local appname="$1"; shift
  local sitename="$1"; shift
  local site_path="$1"; shift

  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  # check if config file already exists and bail if so.
  if [ -f "$site_config" ]; then
    echo "The apache configuration file already exists at:"
    echo "  $site_config"
    echo "Since apache configuration files can get very complex, we do not want to"
    echo "assume that this file is removable.  Calling the site addition done."
    exit 0
  fi

  echo "Creating a new apache2 site for $sitename with config file:"
  echo "  $site_config"

  # if no path, then we default to our standard app storage location.  otherwise, we
  # put the site where they told us to.
  if [ -z "$site_path" ]; then
    # path where site gets checked out, in some arcane manner, and which happens to be
    # above the path where we put webroot (in the storage suffix, if defined).
    local path_above="${BASE_APPLICATION_PATH}/${appname}"
    # no slash between appname and suffix, in case suffix is empty.
    local full_path="${path_above}${STORAGE_SUFFIX}"
#echo really full path is $full_path
  else
    # we'll go with their specification for the site storage.
    local full_path="$site_path"
  fi

  echo "
# set up the user's web folder as an apache user web directory.

# set permissions on the actual app folder.
<Directory \"$full_path\">
  Options +ExecCGI +Indexes +FollowSymLinks +Includes +MultiViews 
  Require all granted
</Directory>

<VirtualHost *:80>
    ServerName ${sitename}
    DocumentRoot ${full_path}
    ErrorLog \${APACHE_LOG_DIR}/${sitename}-error.log
    CustomLog \${APACHE_LOG_DIR}/${sitename}-access.log combined
    Include /etc/apache2/conf-library/basic-options.conf
    Include /etc/apache2/conf-library/rewrite-enabling.conf
</VirtualHost>
" >"$site_config" 

  chown "$(logname):$(logname)" "$site_config"
  test_or_die "setting ownership on: $site_config"
}

# stops apache from serving up the site.
function disable_site()
{
  local sitename="$1"; shift
  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  if [ ! -f "$site_config" ]; then
    echo "The site config did not exist and could not be disabled: $site_config"
    return 0
  fi

#hmmm: repeated pattern of hidden output file, very useful.  abstract it...
  local outfile="$TMP/apacheout.$RANDOM"
  a2dissite "$(basename $site_config)" &>$outfile
  if [ $? -ne 0 ]; then
    # an error happened, so we show the command's output at least.
    cat $outfile
    echo
    echo "There was a problem disabling the apache config file in:"
    echo "  $site_config"
    echo "Please consult the apache error logs for more details."
    exit 1
  fi
  \rm "$outfile"
}

# turns on the config file we create above for apache.
function enable_site()
{
  local sitename="$1"; shift
  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  local outfile="$TMP/apacheout.$RANDOM"
  a2ensite "$(basename $site_config)" &>$outfile
  if [ $? -ne 0 ]; then
    # an error happened, so we show the command's output at least.
    cat $outfile
    echo
    echo "There was a problem enabling the apache config file in:"
    echo "  $site_config"
    echo "Please consult the apache error logs for more details."
    exit 1
  fi
  \rm "$outfile"
}

# restarts the apache2 service.
function restart_apache()
{
  service apache2 restart
  if [ $? -ne 0 ]; then
    echo "There was a problem restarting the apache2 service."
    echo "Please consult the apache error logs for more details."
    exit 1
  fi
  echo Apache2 service restarted.
}

# sets up the serverpilot storage location for a user hosted web site.
function maybe_create_site_storage()
{
  local our_app="$1"; shift
  # make sure the path for storage this app exists for the user.
  local full_path="$BASE_APPLICATION_PATH/$our_app"
  if [ ! -d "$full_path" ]; then
    mkdir -p $full_path
    test_or_die "The app storage path could not be created.\n  Path in question is: $full_path"
  fi

  # now give the web server some access to the folder.  this is crucial since the folders
  # can be hosted in any user folder, and the group permissions will not necessarily be correct already.
  local chow_path="$full_path"
  # only the first chmod is recursive; the rest just apply to the specific folder of interest.
  chmod -R g+rx "$chow_path"
  # walk backwards up the path and fix perms.
  while [[ $chow_path != $HOME ]]; do
#echo chow path is now $chow_path
    chmod g+rx "$chow_path"
    test_or_die "Failed to add group permissions on the path: $chow_path"
    # reassert the user's ownership of any directories we might have just created.
    chown $(logname) "$chow_path"
    test_or_die "changing ownership to user failed on the path: $chow_path"
    chow_path="$(dirname "$chow_path")"
  done
}

############################################################################


