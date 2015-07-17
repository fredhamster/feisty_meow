#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/buildor/gffs_builders.sh

echo stopping any running genesis processes...
bash $XSEDE_TEST_ROOT/library/zap_genesis_javas.sh 

echo building the code freshly, although not with a clean first...
build_gffs 
if [ $? -ne 0 ]; then
  echo failed to build the code.
fi

echo starting container now and spooling its log file...
(bash $XSEDE_TEST_ROOT/library/maybe_restart_container.sh &>$TMP/container_restarty.log & )
tail -f ~/.GenesisII/container.log 
