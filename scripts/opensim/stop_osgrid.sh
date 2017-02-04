#!/bin/bash
# this script stops a 0.6.9 era opensimulator system that was started by
# the run_opensim or run_osgrid script.  this will work for either case.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/opensim/opensim_utils.sh"

# the brains for the script is the screen and process names we need to control.
close_application opensim OpenSim.exe

