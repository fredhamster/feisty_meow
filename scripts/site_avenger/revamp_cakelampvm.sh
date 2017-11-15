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

##############

# fix up the main web storage.
chown -R www-data:www-data /var/www 
test_or_die "chown www-data"
group_perm /var/www 
test_or_die "group_perm www-data"

##############

# set up access on some important folders for the developer user.
chown -R developer:developer /home/developer
test_or_die "chown developer home"
harsh_perm /home/developer/.ssh
test_or_die "harsh_perm setting on developer .ssh"
chown -R developer:developer /etc/apache2 /etc/bind 
test_or_die "chown apache2 and bind to developer"
group_perm /etc/apache2 /etc/bind 
test_or_die "group perms on apache2 and bind"

##############

# fix perms for fred user.
chown -R fred:fred /home/fred /home/archives/stuffing 
test_or_die "chown fred home"
harsh_perm /home/fred/.ssh
test_or_die "harsh_perm setting on fred .ssh"
chown -R fred:fred /opt/feistymeow.org 
test_or_die "chown feisty meow to fred"
group_perm /opt/feistymeow.org 
test_or_die "group perms on feisty meow"

##############
#
# some slightly tricky bits start here.  we want to massage the vm into the
# best possible shape without needing to re-release it.
#
##############

# only update hello if they've still got the file there.  we don't want to
# keep forcing our hellos at people.
if [ -f "$HOME/hello.txt" ]; then
  # copy the most recent hello file into place for the user.
  \cp -f "$FEISTY_MEOW_APEX/production/sites/cakelampvm.com/hello.txt" "$HOME"
  test_or_continue "copying hello file for user"
fi

##############

# install a better editor app.

echo "
The script is about to install the bluefish editor and some dependencies.
This takes about a minute on a not very fast internet connection...

"

apt-get install -y bluefish &> "/tmp/install_bluefish-$(logname).log"
test_or_continue "failed to install bluefish editor.  not good."

##############


#hmmm: todo
# deploy the site updater here to fix the local cakelampvm.com site...


##############

# sequel--tell them they're great and show the hello again also.

regenerate

echo "


Thanks for revamping your cakelampvm.  :-)
"

##############


