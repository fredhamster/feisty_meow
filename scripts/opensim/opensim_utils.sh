#!/bin/bash
# this is a collection of scripts that assist in managing an opensim server.
# it uses the "screen" utility to manage opensimulator instances.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

# set up some parameters that we believe (or have been told) are beneficial.
export MONO_THREADS_PER_CPU=1208

# we run the processes with a little cpu de-prioritization.  we do not want
# them taking over completely if there's a runaway mono tornado.
export NICENESS_LEVEL=6

# a tip that supposedly helps on linux so that there won't be bizarre
# problems compiling scripts.
export LANG=C

# this is used as a process startup snooze, to avoid running a dependent
# process before the dependency has really started.
export SNOOZE_TIME=6

# lock the limit in for threads, so we don't have any getting out of control.
# also make sure we've provided enough space for each thread.
ulimit -s 512144

# use more recent versions of mono for opensim if they're available.
if [ -d /opt/mono-2.10/bin ]; then
  export PATH=/opt/mono-2.10/bin:$PATH
elif [ -d /opt/mono-2.8/bin ]; then
  # use version 2.8 mono for opensim if it's available.
  export PATH=/opt/mono-2.8/bin:$PATH
fi

function launch_screen()
{
  screen_name="$1"; shift
  app_name="$1"; shift
  echo "$(date_stringer ' '): starting $screen_name now..."
  screen -L -S "$screen_name" -d -m nice -n $NICENESS_LEVEL mono "$app_name" 
##why? --debug 
#no, makes it ugly: -console=basic 

  echo "$(date_stringer ' '): $screen_name started."
  # only sleep if we are not at the last process that gets started.
  if [ "$app_name" != "OpenSim.exe" ]; then
    sleep $SNOOZE_TIME
  fi
}

# finds the opensim process specified or returns a blank string in the
# OS_PROC_ID variable.
export OS_PROC_ID=
function find_opensim_process()
{
  OS_PROC_ID=
  process_name="$1"; shift
  if [ -z "$process_name" ]; then
    return 1  # failure in call.
  fi
  OS_PROC_ID=$(ps wuax | grep "[0-9] mono $process_name" | grep -vi screen | sed -e "s/$USER  *\([0-9][0-9]*\).*/\1/" | head -n 1)
}

# takes a screen name for the detached screen session and a process name that
# we should be able to find running.  we make sure that both are shut down.
function close_application()
{
  screen_name="$1"; shift
  process_name="$1"; shift
  echo "$(date_stringer ' '): stopping $screen_name now..."
  screen -r -s "$screen_name" -X quit

  # we don't want to shut any other servers down until this process is really gone.
  find_opensim_process $process_name
  if [ ! -z "$OS_PROC_ID" ]; then
    echo "$(date_stringer ' '): waiting for $screen_name to really shut down..."
    sleep $SNOOZE_TIME
    # check again after the snooze.
    find_opensim_process $process_name
    while [ ! -z "$OS_PROC_ID" ]; do
      find_opensim_process $process_name
#break out on timed basis.
    done
    echo "$(date_stringer ' '): $screen_name really is shut down now."

#do this as last ditch, above in timeout
    find_opensim_process $process_name
    if [ ! -z "$OS_PROC_ID" ]; then
      echo "process for $screen_name still exists, killing $process_name (id $OS_PROC_ID) now."
      kill -9 $OS_PROC_ID
      sleep 2
    fi

  fi

  echo "$(date_stringer ' '): $screen_name stopped."
}

