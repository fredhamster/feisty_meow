#!/bin/bash
perl $FEISTY_MEOW_SCRIPTS/schedule/generate_reminders.pl
echo "(bash $FEISTY_MEOW_SCRIPTS/schedule/start_calendar.sh) &>/dev/null" | at 4:20am

