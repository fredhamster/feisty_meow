
# some helpful functions for counting a duration.

#hmmm: should take a variable name for the start tracker and end tracker.

function start_time_tracking()
{
  local varname="$1"; shift
  if [ -z "$varname" ]; then echo must pass variable name to start_time_tracking; return 1; fi
  local startvar="${varname}_START"
  eval $startvar="$(date +"%s")"
}

function end_time_tracking()
{
  local varname="$1"; shift
  if [ -z "$varname" ]; then echo must pass variable name to end_time_tracking; return 1; fi
  local endvar="${varname}_END"
  eval $endvar="$(date +"%s")"
}

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

echo testing time tracking...
start_time_tracking crungle

sleep $(($RANDOM / 1000))
sleep 2

end_time_tracking crungle

echo ...done testing time tracking.

show_tracked_duration crungle

