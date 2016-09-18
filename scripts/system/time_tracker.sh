#!/bin/bash

# some helpful functions for counting a named duration.

# start tracking time for a named purpose, passed as the first parameter.
function start_time_tracking()
{
  local varname="$1"; shift
  if [ -z "$varname" ]; then echo must pass variable name to start_time_tracking; return 1; fi
  local startvar="${varname}_START"
  eval $startvar="$(date +"%s")"
}

# stop tracking the named time duration.
function end_time_tracking()
{
  local varname="$1"; shift
  if [ -z "$varname" ]; then echo must pass variable name to end_time_tracking; return 1; fi
  local endvar="${varname}_END"
  eval $endvar="$(date +"%s")"
}

# display the time taken for a named duration.
function show_tracked_duration()
{
  varname="$1"; shift
  if [ -z "$varname" ]; then echo must pass variable name to end_time_tracking; return 1; fi
  # calculate the time taken.
  local startvar="${varname}_START"
  local endvar="${varname}_END"
  duration="$((${!endvar} - ${!startvar}))"
  # separate it into hours and minutes.
  minutes="$(($duration / 60))"
  hours="$(($duration / 60))"
  # subtract the hours from the minutes sum for modulo.
  minutes="$(($minutes - $hours * 60))"
  # fashion conceit, add zeroes.
  if (($minutes < 10)); then minutes="0$minutes"; fi
  if (($hours < 10)); then hours="0$hours"; fi
  echo "Time taken for ${varname}: ${hours}:${minutes} hh:mm (a total of $duration seconds)"
}


