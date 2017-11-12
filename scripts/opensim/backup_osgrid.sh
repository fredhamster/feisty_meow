#!/bin/bash
# a simple script that backs up the opensim database assets.

#hmmm: need to parameterize for the database name and secrets file and all that.
#      would be nice to have a block of opensim variables, perhaps an associative
#      array of config chunks.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

host=$(echo $(hostname) | sed -e 's/\([^.]*\)\..*/\1/')

bkupname="backup-osgrid_${host}_$(date_stringer).mysql_bkup"
mysqldump -u griduser -p$(cat $HOME/.secrets/opensim_db_password.txt) opengrid > "$bkupname"
# note that the above assumes the database is called "opensim".  it might be
# called opengrid instead, based on the setup procedure that was followed.
# likewise the user might be someone other than "griduser".
gzip "$bkupname"


