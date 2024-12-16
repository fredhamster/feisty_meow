#!/usr/bin/env bash

# backs up the uva onedrive folder (which itself is fed from the cloud).

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

pushd ~/data
exit_on_error changing to data folder.

zip -r ~/onedrive_backup_uva_$(date_stringer).zip onedrive-uva-live -x \*.ova -x \*.webarchive 
exit_on_error zipping up the uva onedrive folder.


popd


