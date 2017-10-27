#!/bin/bash

# creates a new apache website for a specified domain.

# some convenient defaults for our current usage.

BASEPATH="/var/www"
SHADOWPATH="/srv/users/serverpilot/apps"
STORAGESUFFIX="/public"

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

  local fullpath="${BASEPATH}/${appname}${STORAGESUFFIX}"

  # make the storage directory if it's not already present.
  if [ ! -d "$fullpath" ]; then
    mkdir -p "$fullpath"
    if [ $? -ne 0 ]; then
      echo "Failed to create the storage directory for $appname in"
      echo "the folder: $fullpath"
      exit 1
    fi
  fi

echo "
<VirtualHost *:80>
    ServerName ${sitename}
#    ServerAlias ${sitename} *.${sitename}
    DocumentRoot ${fullpath}
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

# sets up a link to represent the serverpilot storage location, while
# still storing the files under /var/www.
function create_shadow_path()
{
  # make sure there is a symbolic link from the shadow path (that mimics the serverpilot
  # storage set up) to the real storage directory.
  if [ ! -L "$SHADOWPATH" ]; then
    # create the path up to but not including the last component.
    if [ ! -d $(dirname $SHADOWPATH) ]; then
      mkdir -p $(dirname $SHADOWPATH)
      if [ $? -ne 0 ]; then
        echo "The parent of the shadow path could not be created."
        echo "Path in question is: $(dirname $SHADOWPATH)"
        exit 1
      fi
    fi

    ln -s "$BASEPATH" "$SHADOWPATH"
#hmmm: should we be okay with it if it's a real dir, and assume people are happy?
#      this wouldn't work too well if we go plunk down the new thing in /var/www,
#      if they are expecting this tool to totally meld with serverpilot.
    if [ $? -ne 0 ]; then
      echo "The shadow path for mimicking serverpilot could not be created."
      echo "Is there a real directory present for this already?"
      echo "Path in question is: $SHADOWPATH"
      exit 1
    fi
  fi
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

create_shadow_path
write_apache_config "$appname" "$site"
enable_site "$site"
restart_apache

