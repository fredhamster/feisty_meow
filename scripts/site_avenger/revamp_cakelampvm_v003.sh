#!/usr/bin/env bash

# fixes the cakelampvm permissions according to the way.

##############

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

if [[ ! $(hostname) == *cakelampvm* ]]; then
  echo "This script is only designed to be run on the cakelampvm host."
  exit 1
fi

##############

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$THISDIR/../.." && \pwd )"

export NO_HELLO=right
source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
# load dependencies for our script.
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"
source "$FEISTY_MEOW_SCRIPTS/security/password_functions.sh"

##############

# it's a requirement to have sql root password, since we may need some sql db configuration.
load_password /etc/mysql/secret_password mysql_passwd
if [ -z "$mysql_passwd" ]; then
  read_password "Please enter the MySQL root account password:" mysql_passwd
fi
if [ -z "$mysql_passwd" ]; then
  echo "This script must have the sql root password to proceed."
  exit 1
else
  store_password /etc/mysql/secret_password "$mysql_passwd"
fi

##############

sep

echo "Regenerating feisty meow loading dock."

recustomize
exit_on_error "regenerating feisty meow configuration"
chown -R "$(fm_username)":"$(fm_username)" /home/$(fm_username)/.[a-zA-Z0-9]*
exit_on_error "fix after reconfigured as sudo"

##############

# set up some crucial users in the mysql db that we seem to have missed previously.

sep

echo "Adding users to the mysql database."

# note regarding v003 of the revamp script: i am leaving the mysql code
# alone for the moment; it's a good example of using our password, gathered
# above.  (a better example wouldn't pass the password on the command line
# but involving an "expect" script is out of scope currently.)  --fred

#hmmm: good example here for why we need the code that "hides output unless error".
mysql -u root -p"$mysql_passwd" &>/dev/null <<EOF
  create user if not exists 'root'@'%' IDENTIFIED BY '$mysql_passwd';
  grant all privileges on *.* TO 'root'@'%' with grant option;

  create user if not exists 'wampcake'@'%' IDENTIFIED BY 'bakecamp';
  grant all privileges on *.* TO 'wampcake'@'%' with grant option;

  create user if not exists 'lampcake'@'%' IDENTIFIED BY 'bakecamp';
  grant all privileges on *.* TO 'lampcake'@'%' with grant option;
EOF
exit_on_error "configuring root, wampcake and lampcake users on mysql"

##############

sep

echo "Making some important permission changes..."

##############

# fix up the main web storage.
chown -R www-data:www-data /var/www 
exit_on_error "chown www-data"
group_perm /var/www 
exit_on_error "group_perm www-data"

##############

# set up access on some important folders for the developer user.

# set the developer user as uber owner of many things with redeveloper alias.
# (must have run feisty meow "recustomize" command at some point to enable.)
redeveloper
exit_on_error "running redeveloper to fix ownership"

##############

# give the developer control over the apache and bind config files, as well
# as giving the user ownership of the local feisty meow repository.
chown -R developer:developer /etc/apache2 /etc/bind 
exit_on_error "chown apache2 and bind to developer"
group_perm /etc/apache2 /etc/bind 
exit_on_error "group perms on apache2 and bind"
chown -R developer:developer /opt/feistymeow.org 
exit_on_error "chown feisty meow to developer"
group_perm /opt/feistymeow.org 
exit_on_error "group perms on feisty meow"

##############

echo "...done with permission changes."

##############
#
# some slightly tricky bits start here.  we want to massage the vm into the
# best possible shape without needing to re-release it.
#
##############
##############

#thing 1

##############

#thing 2

##############
##############

# sequel--tell them they're great and show the hello again also.

sep

recustomize
exit_on_error "recustomize-ing feisty meow scripts"
chown -R "$(fm_username)":"$(fm_username)" /home/$(fm_username)/.[a-zA-Z0-9]*
exit_on_error "fix after recustomize as sudo"
echo "


Thanks for revamping your cakelampvm.  :-)

You may want to update your current shell's feisty meow environment by typing:
  recustomize
"

##############


