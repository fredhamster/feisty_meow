#!/bin/bash

# provides the default values for the variables used in our site management scripts.

# config files for site avenger apps usually override nothing, since we
# auto-construct the app name and domain.
# if they do need to override anything, they can just specify replacement
# values for the variables in this file.

####

# basic information that is constant for all site avenger sites.

# the top level of the user's application storage.
export BASE_APPLICATION_PATH="$HOME/apps"
# where the code should come from.
export DEFAULT_REPOSITORY_ROOT="git@github.com:kwentworth"
# we checkout the git repository to a directory underneath the
# app storage directory named this:
export CHECKOUT_DIR_NAME="avenger5"
# the subfolder that the web browser will look for the site in,
# underneath the application's specific path.
export STORAGE_SUFFIX="/public"

####

# constants within our cakelampvm machine.

# in our scheme, the single IP address that all our domains map to.
export IP_ADDRESS="10.28.42.20"
# the email address (where first dot is replaced by @) for the administrator of the domain.
export SERVER_ADMIN="developer.cakelampvm.com"
# the name of the name server for the new domains (should already be configured).
export MAIN_NAME_SERVER="ns.cakelampvm.com"
# the name of the mail server for a new domain (should already be configured).
export MAIL_SERVER="mail.cakelampvm.com"
# the distribution name to be listed in info for the new domain or subdomain.
export DISTRO="ubuntu"

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

