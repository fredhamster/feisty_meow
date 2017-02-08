#!/bin/bash

# updates the mounted barkuptree drive with stuff on wildmutt.
# very specific currently.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

rsync -av /z/pooling/archive_backups/* /media/fred/barkuptreedrive/archive_backups/
check_result "synching archive backups"
rsync -av /z/walrus/* /media/fred/barkuptreedrive/walrus/ 
check_result "synching walrus"
rsync -av /z/musix/* /media/fred/barkuptreedrive/musix/
check_result "synching musix"
