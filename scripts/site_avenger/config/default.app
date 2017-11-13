#!/bin/bash

# this config file provides the default values for the variables used in our site management scripts.

####

# basic information that is constant for all site avenger sites.

export APPLICATION_DIR="$HOME/apps"
export DEFAULT_REPOSITORY_ROOT="git@github.com:kwentworth"
export CHECKOUT_DIR_NAME="avenger5"

####

# config files for site avenger apps usually override nothing, since we
# auto-construct the app name and domain.  but if they do need to override
# anything, it will be below this point in the file.
# the derived config file should include the basic configs like so:
#
#   source "$WORKDIR/config/default.app"

####

# deployment information for the application / site.

export APPLICATION_NAME="${app_dirname}"

echo app name was computed as $APPLICATION_NAME

# change this if the site is on the "real" internet.
export DOMAIN_NAME="${app_dirname}.vm"

echo domain name was computed as $DOMAIN_NAME

export REPO_NAME="${app_dirname}"

echo repo name was computed as $REPO_NAME

export THEME_NAME="$(capitalize_first_char "${app_dirname}")"

echo theme name was computed as $THEME_NAME

####

