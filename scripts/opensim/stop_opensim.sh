#!/bin/bash
# this script stops a 0.6.9 era opensimulator system that was started by
# the run_opensim or run_osgrid script.  this will work for either case.

source $SHELLDIR/opensim/opensim_utils.sh

# the brains for the script is the screen and process names we need to control.
close_application opensim OpenSim.exe
close_application robust Robust.exe

