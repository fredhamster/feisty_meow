#!/bin/bash

# creates a new apache website for a specified domain.

# auto-find the scripts, since we might want to run this as sudo.
export WORKDIR="$( \cd "$(\dirname "$0")" && /bin/pwd )"  # obtain the script's working directory.
echo WORKDIR is $WORKDIR
source "$WORKDIR/../core/launch_feisty_meow.sh"

# some convenient defaults for our current usage.

BASE_PATH="$HOME/apps"
STORAGE_SUFFIX="/public"

# this function writes out the new configuration file for the site.
function write_apache_config()
{
  local appname="$1"; shift
  local sitename="$1"; shift
  local site_config="/etc/apache2/sites-available/${sitename}.conf"

  # check if config file already exists and bail if so.
  if [ -f "$site_config" ]; then
    echo "The apache configuration file already exists at:"
    echo "  $site_config"
    echo "Please remove this file before proceeding, if it is junk.  For example:"
    echo "  sudo rm $site_config"
    exit 1
  fi

  echo "Creating a new apache2 site for $sitename with config file:"
  echo "  $site_config"

  local full_path="${BASE_PATH}/${appname}${STORAGE_SUFFIX}"
echo really full path is $full_path

#hmmm: the code below is just getting bigger.  it would be nice to create the chunks of permission stuff
# via iteration rather than hardcoding.

echo "
# we have to enable some directory access through the user's folders.
# this is probably going to end up repeated in multiple apache files, but
# hopefully that's not a problem.
#hmmm: fix above note if it's not a problem.
#
# set permissions on the root folders.
<Directory \"/\">
  Options -ExecCGI +Indexes +FollowSymLinks +Includes
  Order allow,deny
  Allow from all
</Directory>
# set permissions on the root of the home folders.
<Directory \"/home\">
  Options -ExecCGI +Indexes +FollowSymLinks +Includes
  Order allow,deny
  Allow from all
</Directory>
# set permissions on the user's home folder.
<Directory \"$HOME\">
  Options -ExecCGI +Indexes +FollowSymLinks +Includes
  Order allow,deny
  Allow from all
</Directory>
# set permissions on the user's app storage folder.
<Directory \"$BASE_PATH\">
  Options +ExecCGI +Indexes +FollowSymLinks +Includes +MultiViews 
  Order allow,deny
  Allow from all
</Directory>
# set permissions on the actual app folder.
<Directory \"$full_path\">
  Options +ExecCGI +Indexes +FollowSymLinks +Includes +MultiViews 
  Order allow,deny
  Allow from all
</Directory>
<VirtualHost *:80>
    ServerName ${sitename}
#    ServerAlias ${sitename} *.${sitename}
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
  rm "$outfile"
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

# chown folder to group www-data.  can be done without setting a user, right?

# sets up the serverpilot storage location for a user hosted web site.
function maybe_create_site_storage()
{
  local our_app="$1"; shift
  # make sure the base path for storage of all the apps for this user exists.
  local full_path="$BASE_PATH/$our_app"
echo full path is $full_path
  if [ ! -d "$full_path" ]; then
    mkdir -p $full_path
    check_result "The app storage path could not be created.\n  Path in question is: $full_path"
  fi
  # now give the web server some access to the folder.  this is crucial since the folders
  # can be hosted in any user folder, and the group permissions will usually be only for the user.
  chown -R $(logname):www-data "$BASE_PATH"
  check_result "Failed to set www-data as the owner on the path: $full_path"
  # note that web serving will also hose up unless the path to the folder is writable.  so we walk backwards
  # and make sure group access is available.
  local chow_path="$full_path"
  while [[ $chow_path != $HOME ]]; do
echo chow path is now $chow_path
    chmod -R g+rx "$chow_path"
    check_result "Failed to add group permissions for www-data on the path: $chow_path"
    # reassert the user's ownership of any directories we might have just created.
    chown $(logname) "$chow_path"
    check_result "changing ownership to user failed on the path: $chow_path"
echo just chowned $chow_path for user $(logname)
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

if [ -z "$appname" -o -z "$site" ]; then
  echo "This script needs to know (1) the appname (application name) for the new"
  echo "site and (2) the DNS name for the apache virtual host."
  echo "The appname should work as a file-system compatible folder name."
  exit 1
fi

maybe_create_site_storage "$appname"
write_apache_config "$appname" "$site"
enable_site "$site"
restart_apache

