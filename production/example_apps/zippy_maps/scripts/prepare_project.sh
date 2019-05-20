#!/bin/bash

#hmmm: test this and make sure it's still right.
#hmmm: also test that it's inclusive of all the necessary steps for success.

#hmmm: aha, this is definitely missing the necessary link creations still.


# bails if any step fails.
function check_result() {
  if [ $? -ne 0 ]; then
    echo -e "failed on: $*"
    error_sound
    exit 1
  fi
}

THIS_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
pushd $THIS_FOLDER/..

echo running composer update process...
composer update
check_result "updating project with composer"

echo -e "\n\nHave the two config files app.php and config_google.php been configured yet AND has the database for the app been added to mysql? (y/N)"
read line

if [ $line != 'y' -a $line != 'Y' -a $line != 'yes' -a $line != 'YES' ]; then
  echo "Please configure the two config files using their default versions as templates (see the config directory for app.default.php and config_google.default.php)"
  exit 1
fi

echo running migration code to build the database...
./bin/cake migrations migrate
check_result "running the database migrations"

echo making asset symlinks
./bin/cake plugin assets symlink
check_result "symlinking web assets"



