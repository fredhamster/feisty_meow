#!/usr/bin/env bash

# a script that handles synchronization of important assets from the MAJOR_ARCHIVE_SOURCES
# and the SOURCECODE_HIERARCHY_LIST onto a backup drive of some sort.  it will only copy folders
# if there is a target folder of the appropriate name already on the backup medium.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# given a location in the filesystem, we will go to that location and attempt to
# update any revision control repositories stored there to the latest versions.
function update_source_folders()
{
  folder="$1"; shift
  sep
  if [ ! -d "$folder" ]; then
    echo "The folder '$folder' does not exist, so skipping repository update there."
    return;
  fi
  echo getting latest codes in $folder...
  pushd "$folder"
  if [ $? -ne 0 ]; then
    echo Changing to the folder $folder failed.
    return 1
  fi
  bash "$FEISTY_MEOW_SCRIPTS/rev_control/rcheckin.sh"
  if [ $? -ne 0 ]; then
    echo Checking out the latest codes has failed somehow for $folder.
    popd
    return 1
  fi
  popd
  sep
}

# this attempts to copy all the contents in a folder called "from" into a folder
# called "to".  it's a failure for the "from" folder to not exist, but the "to"
# is allowed to not exist (in which case we don't try to synch to it).
function synch_directory_to_target()
{
  local from="$1"; shift
  local to="$1"; shift
  local extra_flag="$1"; shift

  sep

  if [ ! -d "$from" ]; then
    echo "skipping synch on missing source directory: $from"
    return 0
  fi
  if [ ! -d "$to" ]; then
    echo "skipping synch into non-existent target directory $to"
    return 0
  fi

  echo "synching from $from into $to"
  netcp $extra_flag "$from"/* "$to"/
  if [ $? -ne 0 ]; then
    echo "The synchronization of $from into $to has failed."
    return 1
  fi
}

# the uber controller method that does the "hard" work of updating.
# any items from the MAJOR_ARCHIVE_SOURCES that are on the target will be
# updated.  any items found on the target matching the members of the
# SOURCECODE_HIERARCHY_LIST will be treated as code hierarchies and updated.
function update_archive_drive()
{
  local target_folder="$1"; shift  # where we're backing up to.

#echo "remaining parms are: $*"
  # implement a mirroring feature if they use the flag "--mirror".
  # this turns on rsync --delete feature to remove things on the destination
  # that are not on the source.
  local RSYNC_EXTRAS=""
  local mirror_parm="$1"; shift
  if [ "$mirror_parm" == "--mirror" ]; then
    RSYNC_EXTRAS="--delete"
    echo "
NOTE:
Due to the presence of the '--mirror' flag on the command line, we are
adding rsync flags to mirror the source, which removes any files that
are on the destination but not on the source.
"
  fi  

  local currdir  # loop variable.

  sep

  if [ ! -d "$target_folder" -a ! -f "$target_folder" ]; then
    echo "Target '$target_folder' is not available currently; not updating."
    return 1
  fi
  echo Target drive currently has...
  dir "$target_folder"

  # synch all our targets.
  for currdir in $MAJOR_ARCHIVE_SOURCES; do
    synch_directory_to_target "$currdir" "$target_folder/$(basename $currdir)"/ $RSYNC_EXTRAS
  done

  sep

  # update source code if present.
  echo getting latest fred repositories...
  pushd "$target_folder"
  for currdir in $SOURCECODE_HIERARCHY_LIST; do
    update_source_folders $currdir
  done
  
  sep

  echo successfully updated all expected portions of the target drive at:
  echo "  $target_folder"
  echo
  popd
}

# compares one local well-known folder against the similar folder on a
# remote destination.  the first folder is the archive name, with no paths.
# the second paramter is the local path component of the archive, such as
# "/z" if the archives are found in the /z hierarchy.  the third parameter
# is the remote location of the archive, which should just be a host name.
# the fourth parameter is the destination path component to match the local
# path component (since these can differ).
function do_a_folder_compare()
{
  local archname="$1"; shift
  local pathing="$1"; shift
  local dest="$1"; shift
  local destpath="$1"; shift
  if [ -z "$archname" -o -z "$dest" ]; then
    echo "do_a_folder_compare needs an archive name and a destination host."
    return 1
  fi

  if [ -d "/z/$archname" ]; then
    sep 14
    echo "Comparing ${pathing}/${archname} with remote $dest:${destpath}/${archname}"
    compare_dirs ${pathing}/${archname} ${dest}:${destpath}/${archname}
    sep 14
  fi
}

# runs through all the local archives on this host to make sure nothing is
# different when compared with the mainline versions on the specified host.
# the first parameter is the remote version to compare against.  if there is
# a second parameter, it is used as the path on the local machine where the
# comparison should be based (e.g. an archive drive rather than /z/).
function uber_archive_comparator()
{
  local remote_arch="$1"; shift
  if [ -z "$remote_arch" ]; then
    echo uber_archive_comparator needs the remote archive host to compare with.
    return 1
  fi
  local local_place="$1"; shift
  if [ -z "$local_place" ]; then
    local_place="/z"
  fi

  sep 14
  echo "comparing against host '$remote_arch'"
  sep 14

#hmmm: shouldn't this be a list in a variable someplace?
  for archicle in \
    basement \
    imaginations \
    musix \
    toaster \
    walrus \
    ; do
      do_a_folder_compare ${archicle} ${local_place} ${remote_arch} "/z"
  done
}


#hmmm: abstractable piece?  the runtime plug at the end of a library script?
# this block should execute when the script is actually run, rather
# than when it's just being sourced.
if [[ $0 =~ .*general_updater\.sh.* ]]; then
  source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
  exit_on_error "sourcing the feisty meow environment"
  update_archive_drive "${@}"
  exit_on_error "updating archive drive at: $*"
fi

