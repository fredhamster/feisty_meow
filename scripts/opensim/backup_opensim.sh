#!/bin/bash
# a simple script that backs up the opensim database assets.

gridusername="$1"; shift
databasename="$1"; shift

if [ -z "$gridusername" ]; then
  gridusername=griduser
fi
if [ -z "$databasename" ]; then
  databasename=opensim
fi

#hmmm: need to parameterize for the database name and secrets file and all that.
#      would be nice to have a block of opensim variables, perhaps an associative
#      array of config chunks.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

host=$(echo $(hostname) | sed -e 's/\([^.]*\)\..*/\1/')

bkupname="backup-opensim_${host}_$(date_stringer).mysql_bkup"
mysqldump -u "$gridusername" -p$(cat $HOME/.secrets/opensim_db_password.txt) "$databasename" > "$bkupname"
# note that the above assumes the database is called "opensim".  it might be
# called opengrid instead, based on the setup procedure that was followed.
# likewise the user might be someone other than "griduser".
gzip "$bkupname"

