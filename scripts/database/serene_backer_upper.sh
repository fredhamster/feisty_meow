#!/bin/bash

function check_if_failed()
{
  if [ $? -ne 0 ]; then
    echo Step failed: $*
    exit 1
  fi
}

# just undo it first, to try to be sure we know we are mounted properly later.
umount /z/backup &>/dev/null

# now saddle up the backup.
mount /z/backup/
check_if_failed "mounting backup folder"

# we should always be synching to an existing set in there.  make sure they exist.
# for the first ever backup, this is not a good check...
test -d /z/backup/etc -a -d /z/backup/home
check_if_failed "testing presence of prior backup"

synch_files /etc /z/backup/etc/
check_if_failed "synching etc to backup"

synch_files /home /z/backup/home/
check_if_failed "synching home to backup"

umount /z/backup/
check_if_failed "unmounting backup folder"


