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

echo "Regenerating feisty meow loading dock."

reconfigure_feisty_meow
test_or_die "feisty meow reconfiguration"

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

echo " The script is about to install the bluefish editor and some dependencies.
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

  # copy in our new 000 version (which  
  cp $FEISTY_MEOW_APEX/production/sites/cakelampvm.com/rolling/default_page.001/* \
      /etc/apache2/sites-available
  test_or_die "installing new apache default sites"

  # there should only be ours at this version level and with that prefix.
  a2ensite 001-*
  test_or_die "enabling new apache default sites"

  restart_apache
fi

##############

##############

# sequel--tell them they're great and show the hello again also.

echo "


"
regenerate
echo "


Thanks for revamping your cakelampvm.  :-)
"

##############


