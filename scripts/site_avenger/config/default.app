#!/bin/bash

# this config file provides the default values for the variables used in our site management scripts.

####

# basic information that is constant for all site avenger sites.

APPLICATION_DIR="$HOME/apps"
DEFAULT_REPOSITORY_ROOT="git@github.com:kwentworth"
CHECKOUT_DIR_NAME="avenger5"

####

# config files for site avenger apps usually override nothing, since we
# auto-construct the app name and domain.  but if they do need to override
# anything, it will be below this point in the file.
# the derived config file should include the basic configs like so:
#
#   source "$WORKDIR/config/default.app"

####

# deployment information for the application / site.

APPLICATION_NAME="$(basename "$SITE_MANAGEMENT_CONFIG_FILE" .app)"

echo app name was computed as $APPLICATION_NAME

# change this if the site is on the "real" internet.
DOMAIN_NAME="$(basename "$SITE_MANAGEMENT_CONFIG_FILE" .app).vm"

echo domain name was computed as $DOMAIN_NAME

####

