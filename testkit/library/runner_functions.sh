#!/usr/bin/env bash

# assorted useful ways of running executables.
#
# Author: Chris Koeritz

##############

# similar to timed_grid, but for arbitrary commands, and fits in with the timing
# calculators.
function timed_command()
{
  echo "[started timer $(readable_date_string)]"
  $(\which time) -p -o "$TESTKIT_TIMING_FILE" $*
  local retval=$?
  echo "[stopped timer $(readable_date_string)]"
  return $retval
}

##############

# uses the timing file to determine how long the last activity took in
# seconds and then prints out the value.
calculateTimeTaken()
{
  head -n 1 $TESTKIT_TIMING_FILE | awk '{print $2}'
}

# calculates the bandwidth for a transfer.  this takes the elapsed time as
# the first parameter and the size transferred as second parameter.
calculateBandwidth()
{
  local real_time="$1"; shift
  local size="$1"; shift
  # drop down to kilobytes rather than bytes.
  size=$(echo $size / 1024 | $(\which bc) -l)

#echo "time=$real_time size=$size"

  local total_sec="$(echo "$real_time" | awk -Fm '{print $1}'| awk -F. '{print $1}' )"
  local bandwidth=""
  if [[ "$total_sec" =~ .*exited.* ]]; then
    echo "FAILURE: Test run failed in some way; setting total seconds to very large number."
    total_sec="99999999"
  fi
  if [ $total_sec -eq 0 ]; then
    # fake it when we get a math issue where something took less than a second.
    total_sec=1
  fi
  bandwidth="$(echo "scale=3; $size / $total_sec" | $(\which bc) -l)"
  echo "$bandwidth"
}

# a wrapper for calculateBandwidth that prints out a nicer form of the
# bandwidth.  it requires the same parameters as calculateBandwidth.
showBandwidth()
{
  echo "  Bandwidth $(calculateBandwidth $*) kbps"
}

##############

# connects to a host as a particular user and executes a command there.
function run_command_remotely()
{
  if [ $# -lt 3 ]; then
    echo This function connects to a remote host to run a command.  It requires
    echo at least three parameters: the host to connect to, the user name on
    echo that host which supports passwordless logins, and the command to run.
    echo The command to run is the third through Nth parameters.
  fi
  host="$1"; shift
  username="$1"; shift
  # run our expecter to feed commands in, and the last one needs to be exit so we
  # return to the original host.
  OUTFILE="$(mktemp $TMP/ssh_run.XXXXXX)"
  expect $TESTKIT_ROOT/library/ssh_expecter.tcl "$username" "" "$host" "${@}" >"$OUTFILE"
  reval=$?
  # make sure we didn't experience a failure on the other side.
  grep "YO_FAILURE" $OUTFILE &>/dev/null
  if [ $? -eq 0 ]; then
    echo Detected failure running command via ssh.
    ((retval++))
  fi

#debugging
echo ========== output from command ============
cat "$OUTFILE"
echo ===========================================

  rm "$OUTFILE"
  return $retval
}

#testing
#run_command_remotely serene fred "ls /"

##############

