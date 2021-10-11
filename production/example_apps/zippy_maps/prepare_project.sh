#!/bin/bash

# bails if any step fails.
function check_result() {
  if [ $? -ne 0 ]; then
    echo -e "failed on: $*"
    error_sound
    exit 1
  fi
}


echo running composer update process...
composer update
check_result "updating project with composer"

## echo copying in updated cakephp-geo google maps helper code.  this should be unnecessary when they update upstream for static map fix.
## cp -fv ./updates_pending/cakephp-geo/src/View/Helper/GoogleMapHelper.php ./vendor/dereuromark/cakephp-geo/src/View/Helper/GoogleMapHelper.php
## check_result "copying the updated google maps helper file"

echo -e "\n\nHave the two config files app.php and config_google.php been configured yet AND has the database for the app been added to mysql? (y/N)"
read line

if [ $line != 'y' -a $line != 'Y' -a $line != 'yes' -a $line != 'YES' ]; then
  echo "Please configure the two config files using their default versions as templates (see the config directory for app.default.php and config_google.default.php)"
  exit 1
fi

echo running migration code to build the database...
./bin/cake migrations migrate
check_result "running the database migrations"



