#!/bin/bash

# fixes the cakelampvm permissions according to the way.

##############

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

##############

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../../../../.." && \pwd )"

source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"

##############

# make sure we have the appropriate access on a few key folders.
normal_perm /var/www

##############

# change the owner for the web roots to the apache user, www-data.
chown -R www-data:www-data /var/www

# put a couple specific ownerships into play so the appropriate user has full access.
chown -R developer:developer /home/developer /opt/feistymeow.org /etc/apache2 /etc/bind

chown -R fred:fred /home/fred /home/archives/stuffing 

##############

# these directories will be given group permissons that enable web server access.
group_perm /var/www /etc/apache2 /etc/bind

##############

