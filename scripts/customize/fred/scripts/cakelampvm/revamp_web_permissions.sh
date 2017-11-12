
# fixes the cakelampvm permissions according to the way.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

##############

# make sure we have the appropriate access on a few key folders.
normal_perm /var/www

##############

# change the owner for the web roots to the apache user, www-data.
sudo chown -R www-data:www-data /var/www

# put a couple specific ownerships into play so the appropriate user has full access.
sudo chown -R developer:developer /home/developer \
  /etc/apache2 \
  /etc/bind \


sudo chown -R fred:fred	/home/fred \
  /home/archives/stuffing \


##############

# these directories will be given group permissons that enable web server access.
group_perm /var/www/html \
  /opt/feistymeow.org \
  /etc/apache \
  /

##############
