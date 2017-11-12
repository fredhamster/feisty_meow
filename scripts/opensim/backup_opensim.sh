#!/bin/bash
# a simple script that backs up the opensim database assets.

gridusername="$1"; shift
databasename="$1"; shift

if [ -z "$gridusername" -o -z "$databasename" ]; then
  echo "
This script takes two parameters: (1) the user name for the opensim database
and (2) the database name.  It will backup that database by logging into
mysql as the user.  The user's password for mysql must be recorded in a local
file called: \$HOME/.secrets/opensim_db_password.txt"
fi

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

host=$(echo $(hostname) | sed -e 's/\([^.]*\)\..*/\1/')

bkupname="backup-opensim_${host}_$(date_stringer).mysql_bkup"
mysqldump -u "$gridusername" -p$(cat $HOME/.secrets/opensim_db_password.txt) "$databasename" > "$bkupname"
gzip "$bkupname"

