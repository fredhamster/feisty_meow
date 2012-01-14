#!/bin/sh
# a set of useful functions related to time.  one can get the current time as
# an integer, add amounts (in seconds) to the time, and set future times for
# activities to occur.  note that the maximum distance between now and the
# desired future time must be less than 24 hours away, or our system based
# on number of seconds will fail.

# **do not use this method directly**  you want time_as_number() instead.
# gets a number representing the time as hours+minutes+seconds.  the plus
# there indicates concatenation rather than addition.
function time_as_raw_number()
{
  date | sed -e 's/^[^ ]*  *[^ ]*  *[0-9]*  *\([0-9][0-9]\):\([0-9][0-9]\):\([0-9][0-9]\) .*$/\1\2\3/g'
}

# returns a sanitized form of the time which will never inadvertently seem
# like an octal number to bash.  we achieve this by always putting a '1'
# in front of the actual time numbers.
function time_as_number()
{
  echo 1$(time_as_raw_number)
}

# returns a number representing the time in X seconds from now.  this
# number has a '1' prefix (to ensure bash never thinks it's an octal
# number).
function add_seconds_to_now()
{
  addin=$1; shift
  time_now=$(time_as_raw_number)
  secs=10#$(echo $time_now | sed -e 's/.*\(..\)$/\1/')
#echo just secs is $secs
  mins=10#$(echo $time_now | sed -e 's/.*\(..\)..$/\1/')
#echo mins is $mins
  rest=10#$(echo $time_now | sed -e 's/\(.*\)....$/\1/')
#echo rest is $rest
  carry1=10#$(( (secs + addin) / 60))
#echo first carry is $carry1
  secs=$(( (secs + addin) % 60))
  if [[ secs -lt 10 ]]; then secs="0$secs"; fi
#echo secs now is $secs
  carry2=10#$(( (mins + carry1) / 60))
#echo second carry is $carry2
  mins=$(( (mins + carry1) % 60))
  if [[ mins -lt 10 ]]; then mins="0$mins"; fi
#echo mins is now $mins
  rest=$((rest + carry2))
  if [[ rest -gt 23 ]]; then rest="00"; fi
#echo rest now is $rest
  # emit the new value for the time after modding the values.
  # we also add the all important preceding '1' to block octal interpretations.
  echo 1$rest$mins$secs
}

# returns a successful (zero) exit value if it's really time to fire off
# an activity that's supposed to happen at "future_time".  the time should
# have been acquired from time_as_number or add_seconds_to_now.
future_time=
function compare_now_to_future()
{
  curr_time=$(time_as_number)
  if [[ curr_time -ge future_time ]]; then
#    echo "$curr_time: time to fire the activity!!"
    return 0
  else
#    echo "$curr_time: not time to fire yet."
    return 1
  fi
}

# resets the future test time based on the passed-in duration (in seconds).
function reset_future_time()
{
  future_time=$(add_seconds_to_now $1)
}

# runs an activity of some sort if it's time to do so.
# this is mostly an example of using the time comparison features.
function run_activity_if_time()
{
  # this is the length of time between the desired activities.
  local DURATION_BETWEEN_ACTIVITIES=20

  if [ -z "$future_time" ]; then
    # we've never had a time to try the test, so we reset the future time.
    reset_future_time $DURATION_BETWEEN_ACTIVITIES
    return 0
  fi
  if compare_now_to_future; then
    echo "$(date) Activating the activity now...."
    # do something here.
    reset_future_time $DURATION_BETWEEN_REBOOTS
  fi
}

# set this to zero to use in normal library settings as a sourced file.
# set it to non-zero to turn on the debugging/unit test code.
time_functions_testing=0

if [ $time_functions_testing -ne 0 ]; then
  # tests for adding seconds to current time.
  echo time now is $(time_as_number)
  echo time 30 seconds in future: $(add_seconds_to_now 30)
  echo time 45 seconds in future: $(add_seconds_to_now 45)
  echo time 60 seconds in future: $(add_seconds_to_now 60)
  echo time 75 seconds in future: $(add_seconds_to_now 75)
  echo time 1800 seconds in future: $(add_seconds_to_now 1800)

  # test comparing to future time.
  echo "$(date) Starting wait of 23 seconds..."
  reset_future_time 23  # 23 seconds to wait.
  while true; do
    if compare_now_to_future; then
      echo "$(date) Finished waiting 23 seconds(?)."
      break
    fi
    sleep 1  # snooze a bit to avoid busy-looping.
  done
fi


