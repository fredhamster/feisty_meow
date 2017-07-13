#!/bin/bash

# updates the mounted barkuptree drive with stuff on wildmutt.
# very specific currently.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

export BARKY=/media/fred/barkuptreedrive

netcp /z/archons/* $BARKY/bkup_archons/
check_result "synching archons"
netcp /z/walrus/* $BARKY/walrus/ 
check_result "synching walrus"
netcp /z/musix/* $BARKY/musix/
check_result "synching musix"


