
# change the owner for the web roots to the apache user, www-data.
sudo chown -R www-data:www-data		/var/www

# make sure we have the appropriate access on a few key folders.
sudo chmod u+rwx,g+rx 			/var/www

# put a couple specific ownerships into play so the appropriate user has full access.
sudo chown -R developer:developer	/var/www/defaultcake.cakelampvm.com
sudo chown -R fred:fred			/var/www/webwork.repository
## add others here for your own projects.

# these directories will be given group permissons that enable web server access.
DIR_LIST="/var/www/defaultcake.cakelampvm.com /var/www/webwork.repository"

# add in group permissions to allow the web server to serve the pages properly.
for currdir in $DIR_LIST; do
  sudo find $currdir -type d -exec chmod -R u+rwx,g+rwx,o-rwx {} ';'
  sudo find $currdir -type f -exec chmod -R u+rw,g+rw,o-rwx {} ';'
done

