#!/bin/bash

# tests out the time tracking methods.

source $FEISTY_MEOW_SCRIPTS/system/time_tracker.sh

echo testing time tracking...
start_time_tracking crungle

sleep $(($RANDOM / 1000))
sleep 2

end_time_tracking crungle

echo ...done testing time tracking.

show_tracked_duration crungle

