#!/bin/bash
# this locates the main opensim process if possible.  if we cannot find it,
# then the process is restarted.

source $FEISTY_MEOW_SCRIPTS/opensim/opensim_utils.sh

# see if the process is findable.
# (this approach will not work if the process actually freezes up but
# is still present.  we'll never notice the problem.  to catch that, we
# could be checking the last update time on the main log file.)
find_opensim_process OpenSim.exe
if [ -z "$OS_PROC_ID" ]; then
  # jump into the opensim binaries directory so we can run stuff.
  pushd $HOME/opensim/bin &>/dev/null
  launch_screen OpenSim OpenSim.exe 
  echo "$(date): Restarted opensim instance on $(hostname)."
  echo
  popd &>/dev/null
#else echo "$(date): did nothing--opensim is already running."
fi

