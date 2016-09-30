#!/bin/bash

# backs up crucial directories on my server into the allotted backup area.
#
# Author: Chris Koeritz

# tests whether the last action worked or not, and if not, it issues the
# complaint passed as the arguments.
function check_if_failed()
{
  if [ $? -ne 0 ]; then
    echo "Step FAILed: $*"
    return 1
  fi
}

# uses the failure checking function, but actually exits out of the script
# if there was a failure detected.
function exit_if_failed()
{
  check_if_failed $*
  if [ $? -ne 0 ]; then
    exit 1
  fi
}

# given a source and target folder, this synchronizes the source into the target.
function synch_to_backup()
{
  local source="$1"; shift
  local dest="$1"; shift
  if [ -z "$source" -o -z "$dest" ]; then
    echo synch_to_backup function requires a source and a target folder to synch.
    exit 1
  fi
  echo "Synchronizing $source into $dest."
####hmmm: temporary measure until top-level dir bug fixed in synch_files app.
###  if [ ! -d "$dest" ]; then
###    mkdir -p "$dest"
###    if [ $? -ne 0 ]; then
###      echo "FAILed to make target directory: $dest"
###      return 1
###    fi
###  fi
  synch_files "$source" "$dest"
  check_if_failed "synching $source to $dest"
}

##############

# main body of script...

# just undo it first, to try to be sure we know we are mounted properly later.
#NO LONGER USING MOUNT: umount /z/backup &>/dev/null

# now saddle up the backup.
#NO LONGER USING MOUNT: mount /z/backup/
#NO LONGER USING MOUNT: exit_if_failed "mounting backup folder"

# we should always be synching to an existing set in there.  make sure they exist.
# for the first ever backup, this is not a good check...
#test -d /z/backup/etc -a -d /z/backup/home
#exit_if_failed "testing presence of prior backup"

##############

synch_to_backup /etc /z/backup/etc/

##############

for subdir in fred/Maildir git sharedspam svn trac www-data ; do 
  synch_to_backup /home/$subdir /z/backup/home/$subdir
done

##############

synch_to_backup /var/lib/mailman /z/backup/var/lib/mailman
synch_to_backup /var/lib/mysql /z/backup/var/lib/mysql

##############

#NO LONGER USING MOUNT: umount /z/backup/
#NO LONGER USING MOUNT: exit_if_failed "unmounting backup folder"


