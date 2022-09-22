#!/usr/bin/env bash

# this is a simplified feisty meow environment loader that starts
# a subshell within which the environment is available.  exiting
# one time gets back to the shell that ran this script.  (this script
# is designed to be executed, not sourced.)

# Author: Chris Koeritz

ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
export CORE_SCRIPTS_DIR="$(echo "$ORIGINATING_FOLDER" | tr '\\\\' '/' )"
echo core scripts dir is $CORE_SCRIPTS_DIR

THIS_TOOL_NAME="$(basename "$0")"

pushd "$CORE_SCRIPTS_DIR/../.." &>/dev/null
export FEISTY_MEOW_APEX="$(/bin/pwd)"
echo feisty apex is now FEISTY_MEOW_APEX=$FEISTY_MEOW_APEX

#hmmm: actually this should run the reconfigure script first!
#      then we would be semi-bulletproof and wouldn't actually have
#      to make users run that as first step?

bash --init-file <(echo "\
echo "subshell sees feisty meow apex as $FEISTY_MEOW_APEX"; \
bash "$CORE_SCRIPTS_DIR/reconfigure_feisty_meow.sh"; \
if [ $? -ne 0 ]; then echo "error--reconfiguring feisty meow environment failed."; fi; \
source "$CORE_SCRIPTS_DIR/launch_feisty_meow.sh"; \
if [ $? -ne 0 ]; then echo "error--launching feisty meow environment failed."; fi; \
")

popd &> /dev/null

####

