#!/bin/bash
# a simple script that backs up the opensim database assets.

source $SHELLDIR/core/date_stringer.sh

host=$(echo $(hostname) | sed -e 's/\([^.]*\)\..*/\1/')

bkupname=osgrid_bkup_${host}_$(date_stringer).mysql_bkup
mysqldump -u griduser -p$(cat $HOME/.secrets/opensim_db_password.txt) opengrid > $bkupname
# note that the above assumes the database is called "opensim".  it might be
# called opengrid instead, based on the setup procedure that was followed.
# likewise the user might be someone other than "griduser".
gzip $bkupname


