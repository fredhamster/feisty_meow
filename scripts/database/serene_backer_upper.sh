#!/bin/bash

function check_if_failed()
{
  if [ $? -ne 0 ]; then
    echo "Step failed: $*"
    return 1
  fi
}

function exit_if_failed()
{
  check_if_failed
  if [ $? -ne 0 ]; then
    exit 1
  fi
}

# just undo it first, to try to be sure we know we are mounted properly later.
umount /z/backup &>/dev/null

# now saddle up the backup.
mount /z/backup/
exit_if_failed "mounting backup folder"

# we should always be synching to an existing set in there.  make sure they exist.
# for the first ever backup, this is not a good check...
#test -d /z/backup/etc -a -d /z/backup/home
#exit_if_failed "testing presence of prior backup"

##############

synch_files /etc /z/backup/etc/
check_if_failed "synching etc to backup"

##############

synch_files /home/albums /z/backup/home/albums
check_if_failed "synching home/albums to backup"

synch_files /home/deepcore /z/backup/home/deepcore
check_if_failed "synching home/deepcore to backup"

synch_files /home/drupal /z/backup/home/drupal
check_if_failed "synching home/drupal to backup"

synch_files /home/fred /z/backup/home/fred
check_if_failed "synching home/fred to backup"

synch_files /home/git /z/backup/home/git
check_if_failed "synching home/git to backup"

synch_files /home/sharedspam /z/backup/home/sharedspam
check_if_failed "synching home/sharedspam to backup"

synch_files /home/sim /z/backup/home/sim
check_if_failed "synching home/sim to backup"

synch_files /home/svn /z/backup/home/svn
check_if_failed "synching home/svn to backup"

synch_files /home/trac /z/backup/home/trac
check_if_failed "synching home/trac to backup"

##############

synch_files /var/lib/mailman /z/backup/var/lib/mailman
check_if_failed "synching var/lib/mailman to backup"

##############

umount /z/backup/
exit_if_failed "unmounting backup folder"


