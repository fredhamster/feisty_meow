#!/bin/bash
# this script starts up the opensim process for osgrid (0.6.9 era) using the
# screen utility.  note that this will only be useful for an osgrid-attached
# sim server.

source $SHELLDIR/opensim/opensim_utils.sh

# jump into the opensim binaries directory so we can run stuff.
pushd $HOME/opensim/bin &>/dev/null
launch_screen OpenSim OpenSim.exe 
popd &>/dev/null


