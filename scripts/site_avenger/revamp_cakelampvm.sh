#!/bin/bash

# fixes the cakelampvm permissions according to the way.

##############

if [[ $EUID != 0 ]]; then
  echo "This script must be run as root or sudo."
  exit 1
fi

##############

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
export FEISTY_MEOW_APEX="$( \cd "$WORKDIR/../.." && \pwd )"

export NO_HELLO=right
source "$FEISTY_MEOW_APEX/scripts/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/system/common_sysadmin.sh"

##############

# new requirement is to get the sql root password, since we need to do some sql db configuration.
echo -n "Please enter the MySQL root account password: "
read mysql_passwd
if [ -z "$mysql_passwd" ]; then
  echo "This script must have the sql root password to proceed."
  exit 1
fi

##############

echo "Regenerating feisty meow loading dock."

reconfigure_feisty_meow
test_or_die "feisty meow reconfiguration"
chown -R "$(logname)":"$(logname)" /home/$(logname)/.[a-zA-Z0-9]*
test_or_die "fix after reconfigured as sudo"

##############

echo "Making some important permission changes..."

# fix up the main web storage.
chown -R www-data:www-data /var/www 
test_or_die "chown www-data"
group_perm /var/www 
test_or_die "group_perm www-data"

##############

# set up access on some important folders for the developer user.
chown -R developer:developer /home/developer /home/developer/.[a-zA-Z0-9]*
test_or_die "chown developer home"
harsh_perm /home/developer/.ssh
test_or_die "harsh_perm setting on developer .ssh"
chown -R developer:developer /etc/apache2 /etc/bind 
test_or_die "chown apache2 and bind to developer"
group_perm /etc/apache2 /etc/bind 
test_or_die "group perms on apache2 and bind"

##############

# fix perms for fred user.
chown -R fred:fred /home/fred /home/archives/stuffing /home/fred/.[a-zA-Z0-9]*
test_or_die "chown fred home"
group_perm $HOME/apps
test_or_die "group perms on fred's apps"
harsh_perm /home/fred/.ssh
test_or_die "harsh_perm setting on fred .ssh"
chown -R fred:fred /opt/feistymeow.org 
test_or_die "chown feisty meow to fred"
group_perm /opt/feistymeow.org 
test_or_die "group perms on feisty meow"
group_perm /home/fred/apps/mapsdemo
test_or_die "group perms on mapsdemo app"

echo "Done with important permission changes."

##############
#
# some slightly tricky bits start here.  we want to massage the vm into the
# best possible shape without needing to re-release it.
#
##############

echo "Updating developer welcome file."

# only update hello if they've still got the file there.  we don't want to
# keep forcing our hellos at people.
if [ -f "$HOME/hello.txt" ]; then
  # copy the most recent hello file into place for the user.
  \cp -f "$FEISTY_MEOW_APEX/production/sites/cakelampvm.com/hello.txt" "$HOME"
  test_or_continue "copying hello file for user"
fi

##############

# install a better editor app.

echo "The script is about to install the bluefish editor and some dependencies.
If the app is not already installed, then this process takes only about a
minute on a slower home DSL internet connection..."

apt-get install -y bluefish &> "/tmp/install_bluefish-$(logname).log"
test_or_continue "installing bluefish editor"

##############

# deploy any site updates here to the VM's cakelampvm.com site.
#
# we want to upgrade the default apache site to the latest, since the new
# version mirrors the one on the internet (but with green checks instead
# of red X's) and since we also support https on the new default version.
# we can do this again later if needed, by upping the numbers on the apache
# site config files.  our original site was 000 and the new version is 001,
# which we've done as a prefix on the config for some reason.  makes the
# code below easy at least.
if [ -L /etc/apache2/sites-enabled/000-default.conf ]; then
  # the old site is in place still, so let's update that.
  echo "Updating default web sites to latest version."

  a2enmod ssl
  test_or_die "enabling SSL for secure websites"

  restart_apache
  test_or_die "getting SSL loaded in apache"

  a2dissite 000-default
  test_or_die "disabling old apache site"

  rm -f /etc/apache2/sites-available/000-default.conf 
  test_or_die "removing old apache site"

  # copy in our new version of the default page.
#hmmm: would be nice if this worked without mods for any new version, besides just 001.  see apache env var file below for example implem.
  cp -f $FEISTY_MEOW_APEX/production/sites/cakelampvm.com/rolling/default_page.001/* \
      /etc/apache2/sites-available
  test_or_die "installing new apache default sites"

  # there should only be ours at this version level and with that prefix.
  a2ensite 001-*
  test_or_die "enabling new apache default sites"

  restart_apache
fi

##############

# fix up the apache site so that HSTS is disabled.  otherwise we can't view
# the https site for cakelampvm.com once the domain name switch has occurred.

# we operate only on our own specialized tls conf file.  hopefully no one has messed with it besides revamp.
# note the use of the character class :blank: below to match spaces or tabs.
search_replace "^[[:blank:]]*Header always set Strict-Transport-Security.*" "# not good for cakelampvm.com -- Header always set Strict-Transport-Security \"max-age=63072000; includeSubdomains;\"" /etc/apache2/conf-library/tls-enabling.conf
if [ $? -ne 0 ]; then
  echo the apache tls-enabling.conf file seems to have already been patched to disable strict transport security. 
else
  restart_apache
  echo successfully patched the apache tls-enabling.conf file to disable strict transport security. 
fi

##############

# fix up bind so that we think of any address with cakelampvm.com on the end
# as being on the vm.  this is already true for some specific sites, but we
# want the wildcard enabled to ease the use of DNS for windows folks.

grep -q "\*[[:blank:]]*IN A[[:blank:]]*10.28.42.20" /etc/bind/cakelampvm.com.conf
if [ $? -eq 0 ]; then
  # already present.
  echo the bind settings for wildcard domains off of cakelampvm.com seems to already be present. 
else
  echo "
; our bind magic, a wildcard domain, for all other sites with cakelampvm.com
; in the domain.  this forces any other sites besides the ones above to route
; to the actual vm IP address, which currently is singular and very fixated.
*				IN A		10.28.42.20
				IN HINFO	"linux vm" "ubuntu"
" >> /etc/bind/cakelampvm.com.conf
  restart_bind
  echo "successfully added wildcard domains to the cakelampvm.com bind configuration."
fi

##############

# fix samba configuration for (ass-headed) default of read-only in user homes.
# why add a necessary feature if you're just going to cripple it by default?

pattern="[#;][[:blank:]]*read only = yes"
replacement="read only = no"

# we just always do the replacement now, after realizing the sentinel pattern
# was acutally already in the file...  too much subtlety can get one into trouble.
sed -i "0,/$pattern/{s/$pattern/$replacement/}" /etc/samba/smb.conf
test_or_die "patching samba configuration to enable write acccess on user home dirs"
# sweet, looks like that worked...
restart_samba
echo successfully patched the samba configuration to enable writes on user home directories. 

##############

# set up some crucial users in the mysql db that we seem to have missed previously.

mysql -u root -p"$mysql_passwd" <<EOF
  create user 'root'@'%' IDENTIFIED BY '$mysql_passwd';
  grant all privileges on *.* TO 'root'@'%' with grant option;

  create user 'wampcake'@'%' IDENTIFIED BY 'bakecamp';
  grant all privileges on *.* TO 'wampcake'@'%' with grant option;

  create user 'lampcake'@'%' IDENTIFIED BY 'bakecamp';
  grant all privileges on *.* TO 'lampcake'@'%' with grant option;
EOF
test_or_die "configuring root, wampcake and lampcake users on mysql"

##############

# add the latest version of the cakelampvm environment variables for apache.

# drop existing file, if already configured.  ignore errors.
a2disconf env_vars_cakelampvm

# plug in the new version, just stomping anything there.
# note: we only expect to have one version of the env_vars dir at a time in place in feisty...
cp -f $FEISTY_MEOW_APEX/production/sites/cakelampvm.com/rolling/env_vars.*/env_vars_cakelampvm.conf /etc/apache2/conf-available
test_or_die "copying environment variables file into place"

# enable the new version of the config file.
a2enconf env_vars_cakelampvm
test_or_die "enabling the new cakelampvm environment config for apache"

echo Successfully configured the apache2 environment variables needed for cakelampvm.

##############
##############

# sequel--tell them they're great and show the hello again also.

echo "


"
regenerate
test_or_die "regenerating feisty meow scripts"
chown -R "$(logname)":"$(logname)" /home/$(logname)/.[a-zA-Z0-9]*
test_or_die "fix after regenerate as sudo"
echo "


Thanks for revamping your cakelampvm.  :-)
"

##############


