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

regenerate
exit_on_error "regenerating feisty meow configuration"
chown -R "$(fm_username)":"$(fm_username)" /home/$(fm_username)/.[a-zA-Z0-9]*
exit_on_error "fix after reconfigured as sudo"

##############

# set up some crucial users in the mysql db that we seem to have missed previously.

sep

echo "Adding users to the mysql database."

#hmmm: good application for hiding output unless error here.
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
chown -R developer:developer /home/developer /home/developer/.[a-zA-Z0-9]*
exit_on_error "chown developer home"
harsh_perm /home/developer/.ssh
exit_on_error "harsh_perm setting on developer .ssh"


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

# fix perms for fred user.
chown -R fred:fred /home/fred /home/archives/stuffing /home/fred/.[a-zA-Z0-9]*
exit_on_error "chown fred home"

#hmmm: argh, wrong check!  can't check a multi-value if it's a directory or not!!!
if [ -d "$FEISTY_MEOW_REPOS_SCAN" ]; then
  group_perm $FEISTY_MEOW_REPOS_SCAN
  exit_on_error "group perms on fred's apps"
fi
harsh_perm /home/fred/.ssh
exit_on_error "harsh_perm setting on fred .ssh"
group_perm /home/fred/apps/mapsdemo
exit_on_error "group perms on mapsdemo app"

echo "...done with permission changes."

##############
#
# some slightly tricky bits start here.  we want to massage the vm into the
# best possible shape without needing to re-release it.
#
##############

sep

echo "Updating developer welcome file."

# only update hello if they've still got the file there.  we don't want to
# keep forcing our hellos at people.
if [ -f "$HOME/hello.txt" ]; then
  # copy the most recent hello file into place for the user.
  \cp -f "$FEISTY_MEOW_APEX/production/sites/cakelampvm.com/hello.txt" "$HOME"
  continue_on_error "copying hello file for user"
fi

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

  sep

  # the old site is in place still, so let's update that.
  echo "Updating default web sites to latest version."

  a2enmod ssl
  exit_on_error "enabling SSL for secure websites"

  restart_apache
  exit_on_error "getting SSL loaded in apache"

  a2dissite 000-default
  exit_on_error "disabling old apache site"

  rm -f /etc/apache2/sites-available/000-default.conf 
  exit_on_error "removing old apache site"

  # copy in our new version of the default page.
#hmmm: would be nice if this worked without mods for any new version, besides just 001.  see apache env var file below for example implem.
  \cp -f $FEISTY_MEOW_APEX/production/sites/cakelampvm.com/rolling/default_page.001/* \
      /etc/apache2/sites-available
  exit_on_error "installing new apache default sites"

  # there should only be ours at this version level and with that prefix.
  a2ensite 001-*
  exit_on_error "enabling new apache default sites"

  restart_apache
fi

##############

# fix up the apache site so that HSTS is disabled.  otherwise we can't view
# the https site for cakelampvm.com once the domain name switch has occurred.

sep

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

sep

grep -q "\*[[:blank:]]*IN A[[:blank:]]*10.28.42.20" /etc/bind/cakelampvm.com.conf
if [ $? -eq 0 ]; then
  # already present.
  echo the bind settings for wildcard domains off of cakelampvm.com seems to already be present. 
else
  echo "


;;;;;;

; our bind magic, a wildcard domain, for all other sites with cakelampvm.com
; in the domain.  this forces any other sites besides the ones above to route
; to the actual vm IP address, which currently is singular and very fixated.
*				IN A		10.28.42.20
				IN HINFO	\"linux vm\" \"ubuntu\"

;;;;;;



" >> /etc/bind/cakelampvm.com.conf
  restart_bind
  echo "successfully added wildcard domains to the cakelampvm.com bind configuration."
fi

##############

# fix samba configuration for screwy default of read-only in user homes.
# why cripple a necessary feature by default?

sep

pattern="[#;][[:blank:]]*read only = yes"
replacement="read only = no"

# we just always do the replacement now rather than making it conditional,
# after realizing the sentinel pattern was actually already in the file...
# too much subtlety can get one into trouble.
sed -i "0,/$pattern/{s/$pattern/$replacement/}" /etc/samba/smb.conf
exit_on_error "patching samba configuration to enable write acccess on user home dirs"
echo successfully patched the samba configuration to enable writes on user home directories. 

# add in a disabling of the archive bit mapping feature, which hoses up the execute bit
# in an attempt to save the sad old DOS archive bit across the samba connection.
grep -q "map archive" /etc/samba/smb.conf
# if the phrase wasn't found, we need to add it.
if [ $? -ne 0 ]; then
  sed -i "s/\[global\]/\[global\]\n\nmap archive = no/" /etc/samba/smb.conf
  exit_on_error "patching samba configuration to turn off archive bit mapping feature"
  echo Successfully fixed Samba to not use the archive bit mapping feature.
fi

# sweet, looks like that worked...
restart_samba

##############

# add the latest version of the cakelampvm environment variables for apache.

sep

# drop existing file, if already configured.  ignore errors.
a2disconf env_vars_cakelampvm &>/dev/null

# plug in the new version, just stomping anything there.
# note: we only expect to have one version of the env_vars dir at a time in place in feisty...
\cp -f $FEISTY_MEOW_APEX/production/sites/cakelampvm.com/rolling/env_vars.*/env_vars_cakelampvm.conf /etc/apache2/conf-available
exit_on_error "copying environment variables file into place"

# enable the new version of the config file.
a2enconf env_vars_cakelampvm
exit_on_error "enabling the new cakelampvm environment config for apache"

echo Successfully configured the apache2 environment variables needed for cakelampvm.

##############

# add in a swap mount if not already configured.

sep

# we will only add swap now if explicitly asked for it.  this is to avoid creating
# a swap file where the vm is running on an SSD, since that can use up the SSD's lifespan
# too quickly.
if [ ! -z "$ADD_SWAP" ]; then
  echo "Checking existing swap partition configuration.
"

  # check for existing swap.
  free | grep -q "Swap:[[:blank:]]*[1-9][0-9]"
  if [ $? -ne 0 ]; then
    # no swap in current session, so add it.
    echo "Enabling ramdisk swap partition...
"
    add_swap_mount
    echo "
Enabled ramdisk swap partition for current boot session."
  fi

  # the above just gives this session a swap partition, but we want to have
  # the vm boot with one also.

  # check if there is already swap mentioned in the root crontab.  we will get root's
  # crontab below since this script has to run as sudo.
  crontab -l | grep -iq add_swap_mount
  if [ $? -ne 0 ]; then
    # no existing swap setup in crontab, so add it.
    echo "
Adding a boot-time ramdisk swap partition...
"
    # need to do it carefully, since sed won't add lines to a null file.  we thus
    # create a temporary file to do our work in and ignore sed as a tool for this.
    tmpfile="$(mktemp junk.XXXXXX)"
    crontab -l 2>/dev/null >"$tmpfile"
    echo "
# need to explicitly set any variables we will use.
FEISTY_MEOW_APEX=${FEISTY_MEOW_APEX}
# add swap space to increase memory available.
@reboot bash $FEISTY_MEOW_APEX/scripts/system/add_swap_mount.sh
" >>"$tmpfile"
    # now install our new version of the crontab.
    crontab "$tmpfile"
    rm "$tmpfile"

    echo "
Added boot-time ramdisk swap partition to crontab for root."
  fi
fi

##############

sep

echo Adding site avenger packages to composer.
# add in site avenger dependencies so we can build avcore properly.
pushd ~ &>/dev/null
sudo -u $(fm_username) composer config -g repositories.siteavenger composer https://packages.siteavenger.com/
popd &>/dev/null

##############

# make the apache umask set group permissions automatically, so we stop having weird
# permission issues on temp dirs.

sep

grep -q "umask" /etc/apache2/envvars
if [ $? -eq 0 ]; then
  # already present.
  echo the umask configuration for apache already appears to be set.
else
  echo "

# set umask to enable group read/write on files and directories.
umask 002

" >> /etc/apache2/envvars
  restart_apache
  echo "successfully changed apache umask configuration to enable group read/write"
fi

##############
##############

# sequel--tell them they're great and show the hello again also.

sep

regenerate
exit_on_error "regenerating feisty meow scripts"
chown -R "$(fm_username)":"$(fm_username)" /home/$(fm_username)/.[a-zA-Z0-9]*
exit_on_error "fix after regenerate as sudo"
echo "


Thanks for revamping your cakelampvm.  :-)

You may want to update your current shell's feisty meow environment by typing:
  regenerate
"

##############


