#!/bin/bash

# backs up our trac repository.

trac_path="$1"; shift

if [ -z "$trac_path" ]; then
  echo This script needs the path to the trac database.
  exit 1
fi

sep='_'

tar -czf /z/stuffing/archives/trac_bkup_$(date +"%Y$sep%m$sep%d$sep%H%M$sep%S" | tr -d '/\n/').tar.gz "$trac_path" &>$TMP/zz_backup_trac.log


