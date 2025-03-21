#!/usr/bin/env bash

bash $FEISTY_MEOW_SCRIPTS/system/list_packages.sh linux-image |grep "^rc" | grep -v extra | awk '{print $2 }'

bash $FEISTY_MEOW_SCRIPTS/system/list_packages.sh linux-header |grep "^rc" | grep -v extra | awk '{print $2 }'



