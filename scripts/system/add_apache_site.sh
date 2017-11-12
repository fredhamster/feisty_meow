#!/bin/bash

# creates a new apache website for a specified domain.

# auto-find the scripts, since we might want to run this as sudo.
export WORKDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
source "$WORKDIR/../core/launch_feisty_meow.sh"

# some convenient defaults for our current usage.

BASE_PATH="$HOME/apps"
STORAGE_SUFFIX="/public"

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
    local path_above="${BASE_PATH}/${appname}"
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
}

# turns on the config file we create above for apache.
function enable_site()
{
  local sitename="$1"; shift
  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  outfile="$TMP/apacheout.$RANDOM"
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
}

# sets up the serverpilot storage location for a user hosted web site.
function maybe_create_site_storage()
{
  local our_app="$1"; shift
  # make sure the base path for storage of all the apps for this user exists.
  local full_path="$BASE_PATH/$our_app"
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
echo chow path is now $chow_path
    chmod g+rx "$chow_path"
    test_or_die "Failed to add group permissions on the path: $chow_path"
    # reassert the user's ownership of any directories we might have just created.
    chown $(logname) "$chow_path"
    test_or_die "changing ownership to user failed on the path: $chow_path"
    chow_path="$(dirname "$chow_path")"
  done
}

# main body of script.

if (( $EUID != 0 )); then
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
  $BASE_PATH/{app name}/$STORAGE_SUFFIX"
  exit 1
fi

maybe_create_site_storage "$appname"
write_apache_config "$appname" "$site" "$site_path"
enable_site "$site"
restart_apache

