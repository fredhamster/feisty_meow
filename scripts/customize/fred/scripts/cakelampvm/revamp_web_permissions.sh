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

# fix up the main web storage.
chown -R www-data:www-data /var/www 
group_perm /var/www 

##############

# set up access on some important folders for the developer user.
chown -R developer:developer /home/developer /opt/feistymeow.org /etc/apache2 /etc/bind 
normal_perm /opt/feistymeow.org 
# don't want others trying to check feisty meow in.
harsh_perm /opt/feistymeow.org/feisty_meow/.git
harsh_perm /home/developer/.ssh
group_perm /etc/apache2 /etc/bind 

##############

# fix perms for fred user.
chown -R fred:fred /home/fred /home/archives/stuffing 
harsh_perm /home/fred/.ssh

##############

