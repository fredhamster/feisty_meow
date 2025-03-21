#!/usr/bin/env bash

# Author: Chris Koeritz
# Author: Kevin Wentworth

# This contains a bunch of reusable functions that help out in managing websites.

# This script is sourced, and relies on the value of THISDIR, which should
# point at the directory where the site management scripts are stored,
# especially this one.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

export SSM_LOG_FILE="$TMP/$(fm_username)-siteavenger-script.log"

# configure feisty revision control to ignore vendor folders.
export NO_CHECKIN_VENDOR=true

# handles the computation of the base application path and the app dir name.
# this expects to be passed the application directory name, but it will attempt to
# do something intelligent if no name is passed in.
function autoconfigure_paths()
{
  export app_dirname="$1"; shift

  if [ -z "$app_dirname" ]; then
    echo "$(date_stringer): Guessing application dir from local folder."
    app_dirname="$(basename $(\pwd))"
    export BASE_APPLICATION_PATH="$(dirname $(\pwd))"
echo "calculated application dir of '$app_dirname' and"
echo "a base app path of '$BASE_APPLICATION_PATH'"
  fi

  # get our configuration loaded, if we know the config file.
  # if there is none, we will use our default version.
  export SITE_MANAGEMENT_CONFIG_FILE
  if [ -z "$SITE_MANAGEMENT_CONFIG_FILE" ]; then
    SITE_MANAGEMENT_CONFIG_FILE="$THISDIR/config/default.app"
    echo "$(date_stringer): Site management config file was not set.  Using default:" >> "$SSM_LOG_FILE"
    echo "$(date_stringer):   $SITE_MANAGEMENT_CONFIG_FILE" >> "$SSM_LOG_FILE"
  fi

  # load in at least the default version to get us moving.
  source "$SITE_MANAGEMENT_CONFIG_FILE"
  exit_on_error "loading site management configuration from: $SITE_MANAGEMENT_CONFIG_FILE"


echo "after site config file sourced, app dirname now '$app_dirname' and"
echo "base app path now '$BASE_APPLICATION_PATH'"

}

# tests that the main storage folder for apps exists.
# the parameter passed in should be the application directory name (app_dirname), without
# any additional path components.  the script will attempt to auto-configure the application
# base path (above the project folder with app_dirname) and get all the other path variables
# established.
function check_apps_root()
{
  local temp_app_dirname="$1"; shift

echo new call to auto conf func...
  autoconfigure_paths "$temp_app_dirname"
echo after call to auto conf func...

  if [ -z "$BASE_APPLICATION_PATH" ]; then
echo fix this: we had no base app path, what to do now?
exit 1
  fi

  if [ ! -d "$BASE_APPLICATION_PATH" ]; then
    echo "$(date_stringer): Creating the apps directory: $BASE_APPLICATION_PATH" >> "$SSM_LOG_FILE"
    mkdir "$BASE_APPLICATION_PATH"
    exit_on_error "Making apps directory when not already present"
  fi
}

#hmmm: extract to core somewhere...
# locates a parent directory of a certain name, if possible.  returns success
# (as zero) if the directory was found, and failure if there was no parent
# named as requested.  sets a global variable PARENT_DIR_FOUND to the full
# directory name that matched, including the name being sought (but omitting
# any deeper directories than that).
function find_named_parent_dir()
{
  local dir_name_sought="$1"; shift
  # clear any previous global result.
  unset PARENT_DIR_FOUND
  # check for degenerate case of parameter count.
  if [ -z "$dir_name_sought" ]; then
    echo "
find_named_parent_dir: requires a directory name parameter, which will be
sought out above the current directory.  the return value indicates whether
the requested name was found or not.
"
    return 1
  fi
  # signal a failure by default with our return value.
  local retval=1
  # loop upwards in dir hierarchy to find the name.
  while true; do
    local currdir="$(\pwd)"
    if [ "$currdir" == "/" ]; then
      # we climbed out of all subdirs.  this is a failure case.
      retval=1
      break
    fi
    # get the base part of our name to check on success.
    local base="$(basename "$currdir")"
    if [ "$base" == "$dir_name_sought" ]; then
      # yes, that is the right name.  success case.  save our result.
      export PARENT_DIR_FOUND="$currdir"
      retval=0
      break
    fi
    # hop up a directory.
    pushd .. &>/dev/null
  done

  # rollback any directories we pushed.
  while popd &>/dev/null; do true; done

  return $retval
}

# tries to find an appropriate config file for the application.
function locate_config_file()
{
  local app_dirname="$1"; shift

  local configfile="$THISDIR/config/${app_dirname}.app"
  echo "$(date_stringer): config file guessed?: $configfile" >> "$SSM_LOG_FILE"
  if [ ! -f "$configfile" ]; then
    # this is not a good config file.  we can't auto-guess the config.
    echo -e "$(date_stringer): 
There is no specific site configuration file in:
  $configfile
We will continue onward using the default and hope that this project follows
the standard pattern for cakephp projects." >> "$SSM_LOG_FILE"
    # we'll pull in the default config file we set earlier; this will
    # reinitialize some variables based on the app name.
  else
    # they gave us a valid config file.  let's try using it.
    export SITE_MANAGEMENT_CONFIG_FILE="$configfile"
  fi

  # try to load the config.
  source "$SITE_MANAGEMENT_CONFIG_FILE"
  exit_on_error "loading site management configuration from: $SITE_MANAGEMENT_CONFIG_FILE"

  return 0
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
  numdirs=$(count_directories "$appsdir/")

  if [ $numdirs -eq 0 ]; then
    sep
    echo "There are no directories in the application directory:"
    echo "  $appsdir"
    echo "Please create a directory for the site storage, based on the application"
    echo "name that you want to work on.  Or you can just pass the directory name"
    echo "on the command line, e.g.:"
    echo "  $(basename $0) turtle"
    sep
    return 1
  elif [ $numdirs -eq 1 ]; then
    # one directory in apps, so we'll pick that one.
    app_dirname="$(basename $(find "$appsdir" -follow -mindepth 1 -maxdepth 1 -type d) )"
    exit_on_error "Guessing application folder"
  else
    # there's more than one folder in apps...

    # make sure we're allowed to auto-guess the folder name from our current dir.
    if [ -z "$NO_AUTOMATIC_FOLDER_GUESS" ]; then
      # if we can find the special checkout directory name above our current PWD, then that
      # might tell us our project name.
      if find_named_parent_dir "$CHECKOUT_DIR_NAME"; then
        # we can grab a name above the checkout dir name location.  let's try that.
        app_dirname="$(basename "$(dirname $PARENT_DIR_FOUND)" )"
      fi
    else
      # flag maintenance, to avoid hosing other commands by leaving this set.
      unset NO_AUTOMATIC_FOLDER_GUESS

      # well, we couldn't guess a directory based on our current location,
      # so ask the user to choose.
      # Reference: https://askubuntu.com/questions/1705/how-can-i-create-a-select-menu-in-a-shell-script
      holdps3="$PS3"
      PS3='Please pick a folder for site initialization: '
      options=( $(find "$appsdir" -follow -mindepth 1 -maxdepth 1 -type d -exec basename {} ';') "Quit")
      select app_dirname in "${options[@]}"; do
        case $app_dirname in
          "Quit") echo ; echo "Quitting from the script."; return 1; ;;
          *) echo ; echo "You picked folder '$app_dirname'" ; break; ;;
        esac
      done
      if [ -z "$app_dirname" ]; then
        echo "The folder was not provided.  This script needs a directory name"
        echo "within which to initialize the site."
        return 1
      fi
      PS3="$holdps3"
    fi
  fi
  test_app_folder "$appsdir" "$app_dirname"
  exit_on_error "Testing application folder: $app_dirname"

  echo "Application folder is: $app_dirname"
  return 0
}

# ensures that the app directory name is valid and then loads the config
# for the app (either via a specific file or using the defaults).
function test_app_folder()
{
  local appsdir="$1"; shift
  local dir="$1"; shift

  local combo="$appsdir/$dir"
  if [ "$dir" == " " ]; then
    # trickery here means we don't expect an intermediate directory component.
    combo="$appsdir"
  fi

  if [ ! -d "$combo" ]; then
    # the directory wasn't there yet, so we will auto-create it.  this should
    # hopefully be the right decision usually.
    echo "$(date_stringer): Creating app directory: $combo" >> "$SSM_LOG_FILE"
    mkdir "$combo"
    exit_on_error "Making application directory when not already present"
  else
    # the directory does exist.  let's test out a theory that it might not be
    # an official site avenger style folder, in which case we need to patch a
    # variable to set expectations.
    if [ ! -d "$combo/$CHECKOUT_DIR_NAME" ]; then
      echo "Dropping expectation for intermediate checkout directory name."
      CHECKOUT_DIR_NAME=" "
    fi
  fi

echo yo modulopius on the variables:
var combo CHECKOUT_DIR_NAME

  locate_config_file "$dir"
}

# eases some permissions to enable apache to write log files and do other shopkeeping.
function fix_site_perms()
{
  local site_dir="$1"; shift

  if [ -f "$site_dir/bin/cake" ]; then
    sudo chmod -R a+rx "$site_dir/bin/cake"
    exit_on_error "Enabling execute bit on cake binary"
  fi

  if [ -d "$site_dir/logs" ]; then
    sudo chmod -R g+w "$site_dir/logs"
    exit_on_error "Enabling group write on site's Logs directory"
  fi

  if [ -d "$site_dir/tmp" ]; then
    sudo chmod -R g+w "$site_dir/tmp"
    exit_on_error "Enabling group write on site's tmp directory"
  fi
}

# tosses out any cached object data that originated from the database.
function clear_orm_cache()
{
  local site_dir="$1"; shift

  if [ -f "$site_dir/bin/cake" ]; then
    # flush any cached objects from db.
    "$site_dir/bin/cake" orm_cache clear
    exit_on_error "Clearing ORM cache"
  fi
}

# checks that the directory provided is a valid git repository.
function is_valid_git_repo()
{
  local complete_path="$1"; shift
  
  # see if the directory even exists.
  if [ ! -d "$complete_path" ]; then
    # nope, that's not a git repo since it's not even there.
    false
    return
  fi

  # directory exists, so let's test it out.
  pushd "$complete_path" &>/dev/null
  exit_on_error "Switching to directory for check out: $complete_path"

  # ask for repository name (without .git).
  if git rev-parse --git-dir > /dev/null 2>&1; then
    # this is a valid git repo.
    true
    return
  fi
 
  # no, this is not a valid git repository.
  popd &>/dev/null
  false
}

# updates the revision control repository passed in.  this expects that the
# repo will live in a folder called "checkout_dirname" under the app path,
# which is the standard for deployed site avenger sites.  if that directory is
# missing, then we assume a checkout of the top-level repository instead.
# important: this also sets a global variable called site_store_path to the full
# path of the application.
function update_repo()
{
  local full_app_dir="$1"; shift
  local checkout_dirname="$1"; shift
  local repo_root="$1"; shift
  local repo_name="$1"; shift

echo "$(date_stringer): here are parms in update repo:" >> "$SSM_LOG_FILE"
echo "$(date_stringer): $(var full_app_dir checkout_dirname repo_root repo_name)" >> "$SSM_LOG_FILE"

  # forget any prior value, since we are going to validate the path.
  unset site_store_path

#  pushd "$full_app_dir" &>/dev/null
#  exit_on_error "Switching to our app dir '$full_app_dir'"

  local complete_path="$full_app_dir"
  if [ ! "$checkout_dirname" == " " ]; then
    # make the full path using the non-empty checkout dir name.
    complete_path+="/$checkout_dirname"
  fi

echo set complete_path: $complete_path
  # store the local version into our special global.
  site_store_path="$complete_path"

  # check out the directory to see if it's a git repository.
  if ! is_valid_git_repo "$complete_path"; then
    if [ -d "$complete_path" ]; then
      # we don't consider the state of having the dir exist but the repo be wrong as good.
      echo "There is a problem; this folder is not a valid repository:"
      echo "  $complete_path"
      echo "This script cannot continue unless the git repository is valid."
      exit 1
    fi
    # okay, so the directory doesn't even exist.  that means we will try to
    # clone the project anew.
    mkdir "$complete_path"
    exit_on_error "Making project directory prior to new clone: $complete_path"
    pushd "$complete_path/.." &>/dev/null
    exit_on_error "Switching to parent directory prior to new clone: $complete_path/.."
    echo "Cloning repository $repo_name now."
    git clone "$repo_root/$repo_name.git" $checkout_dirname
    exit_on_error "Git clone of repository: $repo_name"
    popd &>/dev/null
  fi

  # a repository was found, so update the version here and leave.
  pushd "$complete_path" &>/dev/null
  exit_on_error "Switching to directory for repo update: $complete_path"
  echo "Repository $repo_name exists.  Updating it."
  git pull --tags --all
  exit_on_error "Recursive checkout on: $complete_path"
  popd &>/dev/null
}

# this function goes to the directory specified and makes it right with
# composer install.  this is as opposed to composer update, which could
# change the state. 
function composer_repuff()
{
  local site_store_path="$1"; shift

  pushd "$site_store_path" &>/dev/null
  exit_on_error "Switching to our app dir '$site_store_path'"

  echo "Updating site with composer..."

  composer -n install
  exit_on_error "Composer installation step on '$site_store_path'."
  echo "Site updated."

#hmmm: untested, had wrong path here and was never being run.
  dir="vendor/siteavenger/avcore"
  if [ -d "$dir" ]; then
    echo "Running avcore database migrations..."
    logfile="$TMP/problem-avcore_db_migration-$(date_stringer).log"
    ./bin/cake migrations migrate -p Avcore &>"$logfile"
    if [ $? -ne 0 ]; then
      echo "** FAILED: Database migrations for avcore.  Check log file in: $logfile"
      # we keep going, because some sites aren't ready for this yet.
    else
      rm "$logfile"
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
  exit_on_error "Switching to our app dir '$site_store_path'"

  pushd webroot &>/dev/null
  exit_on_error "Switching to our webroot dir"

  # remove all symlinks that might plague us.
  find . -maxdepth 1 -type l -exec rm -f {} ';'
  exit_on_error "Cleaning out links in webroot"

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

  # hop up a level above where we had been.  this is a level above the
  # site store path, which will not be appropriate for all projects, so
  # we must tread carefully.
  pushd .. &>/dev/null

  # we only do the following linking exercises when we are sure this is a
  # site avenger style application.  otherwise we would be creating links
  # above our own heads, sort of.
  if [ -d "$CHECKOUT_DIR_NAME" ]; then
    # link 'public' to webroot.
    if [ -L public ]; then
      # public is a symlink.
      rm public
      exit_on_error "Removing public directory symlink"
    elif [ -d public ]; then
      # public is a folder with default files.
#hmmm: is that safe?
      rm -rf public
      exit_on_error "Removing public directory and contents"
    fi

    # create the main 'public' symlink
#hmmm: argh global
    make_safe_link $CHECKOUT_DIR_NAME/webroot public
    exit_on_error "Creating link to webroot called 'public'"
#hmmm: public/$themelower/im will be created automatically by system user with appropriate permissions

  else
    echo "Skipping 'public' link for project without '$CHECKOUT_DIR_NAME' folder."
  fi

  popd &>/dev/null
  popd &>/dev/null

  echo Created symbolic links.
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
  user_name="$(sanitized_username)"
  if [ ! -z "$user_name" -a "$user_name" != "root" ]; then
    echo "$(date_stringer): Chowning the app folder to be owned by: $user_name" >> "$SSM_LOG_FILE"
#hmmm: have to hope for now for standard group named after user 
    sudo chown -R "$user_name:$user_name" "$combo"
    exit_on_error "Chowning $combo to be owned by $user_name"
  else
    echo "$(date_stringer): user name failed checks for chowning, was found as '$user_name'" >> "$SSM_LOG_FILE"
  fi

#hmmm: is this variable set by this point?  it's the right thing to pass down there anyway.
  fix_site_perms "$site_store_path"
}

# Jumps to an application directory given the app name.  If no app name is
# given, it will show a menu to pick the app.
function switch_to()
{
  # check for parameters.
  app_dirname="$1"; shift

  check_apps_root "$app_dirname"

  # find proper webroot where the site will be initialized.
  if [ -z "$app_dirname" ]; then
    # no dir was passed, so guess it.
    export NO_AUTOMATIC_FOLDER_GUESS=true
    find_app_folder "$BASE_APPLICATION_PATH"
  else
    test_app_folder "$BASE_APPLICATION_PATH" "$app_dirname"
  fi
  if [ $? -ne 0 ]; then
    if [ "$app_dirname" != "Quit" ]; then
      echo "Could not locate the application directory: ${app_dirname}"
    fi
    return 1
  fi

  # where we expect to find our checkout folder underneath.
  full_app_dir="$BASE_APPLICATION_PATH/$app_dirname"

  pushd $full_app_dir/$CHECKOUT_DIR_NAME
#redundant if pushd  pwd
}

