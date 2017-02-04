#!/bin/bash
# this script starts up all the opensim processes (0.6.9 era) using the screen
# utility.  then the processes can all be accessed when desired, rather than
# needing to be started in 5 or so separate windows.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/opensim/opensim_utils.sh"

# jump into the opensim binaries directory so we can run stuff.
pushd $HOME/opensim/bin &>/dev/null

launch_screen robust Robust.exe
launch_screen opensim OpenSim.exe 

popd &>/dev/null


