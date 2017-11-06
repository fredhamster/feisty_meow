#!/bin/bash

# updates the mounted barkuptree drive with stuff on wildmutt.
# very specific currently.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

export BARKY=/media/fred/barkuptreedrive

# copy up the archived bluray discs, and possibly future archived formats.
netcp /z/archons/* $BARKY/bkup_archons/
check_result "synching archons"

# copy over our somewhat attenuated but still important walrus archives.
netcp /z/walrus/* $BARKY/walrus/ 
check_result "synching walrus"

# copy all the music files for future reference.
netcp /z/musix/* $BARKY/musix/
check_result "synching musix"

# back up the photo archives.
netcp /z/imaginations/* $BARKY/imaginations/
check_result "synching imaginations"


