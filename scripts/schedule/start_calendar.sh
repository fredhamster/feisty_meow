#!/bin/bash
perl $SHELLDIR/schedule/generate_reminders.pl
echo '(export LIBDIR=$LIBDIR; bash $SHELLDIR/schedule/start_calendar.sh) &>/dev/null' | at 4:20am

