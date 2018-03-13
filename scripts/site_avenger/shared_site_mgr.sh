#!/bin/bash

# Author: Kevin Wentworth
# Author: Chris Koeritz

# This contains a bunch of reusable functions that help out in managing websites.

# This script is sourced, and relies on the value of WORKDIR, which should
# point at the directory where the site management scripts are stored,
# especially this one.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# get our configuration loaded, if we know the config file.
# if there is none, we will use our default version.
export SITE_MANAGEMENT_CONFIG_FILE
if [ -z "$SITE_MANAGEMENT_CONFIG_FILE" ]; then
  SITE_MANAGEMENT_CONFIG_FILE="$WORKDIR/config/default.app"
  echo "Site management config file was not set.  Using default:"
  echo "  $SITE_MANAGEMENT_CONFIG_FILE"
fi

# load in at least the default version to get us moving.
source "$SITE_MANAGEMENT_CONFIG_FILE"
test_or_die "loading site management configuration from: $SITE_MANAGEMENT_CONFIG_FILE"

# configure feisty revision control to ignore vendor folders.
export NO_CHECKIN_VENDOR=true

# tests that the main storage folder for apps exists.
function check_application_dir()
{
  local appdir="$1"; shift
  if [ ! -d "$appdir" ]; then
    echo "Creating the apps directory: $appdir"
    mkdir "$appdir"
    test_or_die "Making apps directory when not already present"
  fi
}

# tries to find an appropriate config file for the application.
function locate_config_file()
{
  local app_dirname="$1"; shift

  local configfile="$WORKDIR/config/${app_dirname}.app"
  echo "config file?: $configfile"
  if [ ! -f "$configfile" ]; then
    # this is not a good config file.  we can't auto-guess the config.
    echo -e "
There is no specific site configuration file in:
  $configfile
We will continue onward using the default and hope that this project follows
the standard pattern for cakephp projects."
    # we'll pull in the default config file we set earlier; this will
    # reinitialize some variables based on the app name.
  else
    # they gave us a valid config file.  let's try using it.
    export SITE_MANAGEMENT_CONFIG_FILE="$configfile"
  fi

  # try to load the config.
  source "$SITE_MANAGEMENT_CONFIG_FILE"
  test_or_die "loading site management configuration from: $SITE_MANAGEMENT_CONFIG_FILE"

}

# this function will seek out top-level directories in the target directory passed in.
# if there is only one directory, then it is returned (in the app_dirname variable).
# otherwise, the user is asked which directory to use.
# important: this sets a global variable app_dirname to the application's directory name.
function find_app_folder()
{
  local appsdir="$1"; shift

  # throw away any prior value so no confusion arises.
  unset app_dirname
  
  # count number of directories...  if exactly one, then choose it.
  numdirs=$(count_directories "$appsdir")

  if [ $numdirs -eq 0 ]; then
    sep
    echo "There are no directories in the application directory:"
    echo "  $appsdir"
    echo "Please create a directory for the site storage, based on the application"
    echo "name that you want to work on.  Or you can just pass the directory name"
    echo "on the command line, e.g.:"
    echo "  $(basename $0) turtle"
    sep
    exit 1
  elif [ $numdirs -eq 1 ]; then
    app_dirname="$(basename $(find "$appsdir" -mindepth 1 -maxdepth 1 -type d) )"
    test_or_die "Guessing application folder"
  else
    # if more than one folder, force user to choose.
    # Reference: https://askubuntu.com/questions/1705/how-can-i-create-a-select-menu-in-a-shell-script
    holdps3="$PS3"
    PS3='Please pick a folder for site initialization: '
    options=( $(find "$appsdir" -mindepth 1 -maxdepth 1 -type d -exec basename {} ';') "Quit")
    select app_dirname in "${options[@]}"; do
      case $app_dirname in
        "Quit") echo ; echo "Quitting from the script."; exit 1; ;;
        *) echo ; echo "You picked folder '$app_dirname'" ; break; ;;
      esac
    done
    if [ -z "$app_dirname" ]; then
      echo "The folder was not provided.  This script needs a directory name"
      echo "within which to initialize the site."
      exit 1
    fi
    PS3="$holdps3"
  fi
  test_app_folder "$appsdir" "$app_dirname"
  test_or_die "Testing application folder: $app_dirname"

  echo "Application folder is: $app_dirname"
}

# ensures that the app directory name is valid.
function test_app_folder()
{
  local appsdir="$1"; shift
  local dir="$1"; shift

  local combo="$appsdir/$dir"

  if [ ! -d "$combo" ]; then
    echo "Creating app directory: $combo"
    mkdir "$combo"
    test_or_die "Making application directory when not already present"
  fi

  locate_config_file "$dir"
}

# eases some permissions to enable apache to write log files and do other shopkeeping.
function fix_site_perms()
{
  local app_dir="$1"; shift

  local site_dir="$app_dir/$CHECKOUT_DIR_NAME"

  if [ -f "$site_dir/bin/cake" ]; then
    chmod -R a+rx "$site_dir/bin/cake"
    test_or_die "Enabling execute bit on cake binary"
  fi

  if [ -d "$site_dir/logs" ]; then
    chmod -R g+w "$site_dir/logs"
    test_or_die "Enabling group write on site's Logs directory"
  fi

  if [ -d "$site_dir/tmp" ]; then
    chmod -R g+w "$site_dir/tmp"
    test_or_die "Enabling group write on site's tmp directory"
  fi
}

# tosses out any cached object data that originated from the database.
function clear_orm_cache()
{
  local site_dir="$1"; shift

  if [ -f "$site_dir/bin/cake" ]; then
    # flush any cached objects from db.
    "$site_dir/bin/cake" orm_cache clear
    test_or_die "Clearing ORM cache"
  fi
}

# updates the revision control repository passed in.  this expects that the
# repo will live in a folder called "checkout_dirname" under the app path,
# which is the standard for our deployed sites.
# important: this also sets a global variable called site_store_path to the full
# path of the application.
function update_repo()
{
  local full_app_dir="$1"; shift
  local checkout_dirname="$1"; shift
  local repo_root="$1"; shift
  local repo_name="$1"; shift

echo here are parms in update repo:
var full_app_dir checkout_dirname repo_root repo_name

  # forget any prior value, since we are going to validate the path.
  unset site_store_path

  pushd "$full_app_dir" &>/dev/null
  test_or_die "Switching to our app dir '$full_app_dir'"

  local complete_path="$full_app_dir/$checkout_dirname"

  # see if the checkout directory exits.  the repo_found variable is set to
  # non-empty if we find it and it's a valid git repo.
  repo_found=
  if [ -d "$checkout_dirname" ]; then
    # checkout directory exists, so let's check it.
    pushd "$checkout_dirname" &>/dev/null
    test_or_die "Switching to our checkout directory: $checkout_dirname"

    # ask for repository name (without .git).
    if git rev-parse --git-dir > /dev/null 2>&1; then
      # this is a valid git repo.
      repo_found=yes
    fi
 
    # we don't consider the state of having the dir exist but the repo be wrong as good.
    if [ -z "$repo_found" ]; then
      echo "There is a problem; this folder is not a valid repository:"
      echo "  $full_app_dir"
      echo "This script cannot continue unless the git repository is valid."
      exit 1
    fi
    popd &>/dev/null
  fi

  if [ ! -z "$repo_found" ]; then
    # a repository was found, so update the version here and leave.
    echo "Repository $repo_name exists.  Updating it."
    rgetem
    test_or_die "Recursive checkout on: $complete_path"
  else
    # clone the repo since it wasn't found.
    echo "Cloning repository $repo_name now."
    git clone "$repo_root/$repo_name.git" $checkout_dirname
    test_or_die "Git clone of repository: $repo_name"
  fi

#not doing this here since powerup uses this and has no sudo.
  #fix_site_perms "$complete_path"

#unused?
  # construct the full path to where the app will actually live.
  site_store_path="$complete_path"

  popd &>/dev/null
}

# this function goes to the directory specified and makes it right with
# composer install.  this is as opposed to composer update, which could
# change the state. 
function composer_repuff()
{
  local site_store_path="$1"; shift

  pushd "$site_store_path" &>/dev/null
  test_or_die "Switching to our app dir '$site_store_path'"

  echo "Updating site with composer..."

  composer -n install
  test_or_die "Composer installation step on '$site_store_path'."
  echo "Site updated."

#hmmm: argh global
  dir="$site_store_path/$CHECKOUT_DIR_NAME/vendor/siteavenger/avcore"
  if [ -d "$dir" ]; then
    echo "Running avcore database migrations..."
    logfile="$TMP/problem-avcore_db_migration-$(date_stringer).log"
    ./bin/cake migrations migrate -p Avcore &>"$logfile"
    if [ $? -ne 0 ]; then
      echo "** FAILED: Database migrations for avcore.  Check log file in: $logfile"
      # we keep going, because some sites aren't ready for this yet.
    else
      \rm "$logfile"
      echo "Database for avcore migrated."
    fi
  fi

  clear_orm_cache

  popd &>/dev/null
}

# this function creates the links needed to make the site function properly given our
# various dependencies and infrastructure.
function create_site_links()
{
  local site_store_path="$1"; shift
  local theme_name="$1"; shift

  echo "Creating symbolic links for site assets..."

  # jump into the site path so we can start making relative links.
  pushd "$site_store_path" &>/dev/null
  test_or_die "Switching to our app dir '$site_store_path'"

  pushd webroot &>/dev/null

  # remove all symlinks that might plague us.
  find . -maxdepth 1 -type l -exec rm -f {} ';'
  test_or_die "Cleaning out links in webroot"

  # link in the avcore plugin.
  make_safe_link "../vendor/siteavenger/avcore/webroot" avcore

  # make the link for our theme as a lower-case version of the theme.
  themelower=${theme_name,,}
  make_safe_link "../plugins/$theme_name/webroot" "$themelower"

  # link in any favicon files.
  if [ -d "../plugins/$theme_name/favicon" ]; then
    local fave
    for fave in "../plugins/$theme_name/favicon"/*; do
      make_safe_link "$fave" .
    done
  fi

  # get back out of webroot.
  popd &>/dev/null

  # hop up a level above where we had been.
  pushd .. &>/dev/null

  # link 'public' to webroot.
  if [ -L public ]; then
    # public is a symlink.
    \rm public
    test_or_die "Removing public directory symlink"
  elif [ -d public ]; then
    # public is a folder with default files.
#hmmm: is that safe?
    \rm -rf public
    test_or_die "Removing public directory and contents"
  fi

  # create the main 'public' symlink
#hmmm: argh global
  make_safe_link $CHECKOUT_DIR_NAME/webroot public
  test_or_die "Creating link to webroot called 'public'"

#hmmm: public/$themelower/im will be created automatically by system user with appropriate permissions

  echo Created symbolic links.

  popd &>/dev/null
  popd &>/dev/null
}

# fetches composer to make sure it's up to date.
# (if powerup runs, composer install doesn't update git origin.)
function update_composer_repository()
{
  local site_store_path="$1"; shift

  pushd "$site_store_path" &>/dev/null

  if git config remote.composer.url &>/dev/null; then
    git pull composer
    echo "Updated the composer repository."
  else
    echo "No composer repository was found for updating."
  fi
}

# fixes the ownership for a site avenger or php application.
# this almost certainly will require sudo capability, if there are any ownership problems
# that need to be resolved.
function fix_appdir_ownership()
{
  local appsdir="$1"; shift
  local dir="$1"; shift

  local combo="$appsdir/$dir"

  # go with the default user running the script.
  user_name="$USER"
  if [ ! -z "$user_name" -a "$user_name" != "root" ]; then
    echo "Chowning the app folder to be owned by: $user_name"
#hmmm: have to hope for now for standard group named after user 
    chown -R "$user_name:$user_name" "$combo"
    test_or_die "Chowning $combo to be owned by $user_name"
  else
    echo "user name failed checks for chowning, was found as '$user_name'"
  fi

  # 
#probably not enough for path!
  fix_site_perms "$combo"
}


