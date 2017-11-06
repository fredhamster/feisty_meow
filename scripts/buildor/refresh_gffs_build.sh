#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/buildor/gffs_builders.sh"

echo stopping any running genesis processes...
bash $GFFS_TOOLKIT_ROOT/library/zap_genesis_javas.sh 

export GFFS_LOGS="$HOME/.GenesisII"

echo cleaning out the logs directory...
\rm -f "$GFFS_LOGS"/*log*

echo making a simple starting log file for container...
if [ ! -d "$GFFS_LOGS" ]; then
  mkdir -p "$GFFS_LOGS"
  test_or_fail Making GFFS logs directory.
fi

echo building the code freshly, although not with a clean first...
build_gffs 
test_or_fail Building GFFS source code.

echo starting container now and spooling its log file...
(bash $GFFS_TOOLKIT_ROOT/library/maybe_restart_container.sh &>$TMP/main_container_restarting.log & )
# snooze a bit so the container gets a chance to write something.
sleep 4
tail -f ~/.GenesisII/container.log 
