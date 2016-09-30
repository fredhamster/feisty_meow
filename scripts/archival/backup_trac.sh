#!/bin/bash

# backs up our trac repository.

trac_path="$1"; shift
archive_path="$1"; shift

if [ -z "$trac_path" -o -z "$archive_path" ]; then
  echo This script needs the path to the trac database as the first parameter
  echo and the path to the storage directory as the second parameter.
  exit 1
fi

sep='_'

tar -czf "${archive_path}/trac_bkup_$(date +"%Y$sep%m$sep%d$sep%H%M$sep%S" | tr -d '/\n/').tar.gz" "$trac_path" &>$TMP/zz_backup_trac.log


