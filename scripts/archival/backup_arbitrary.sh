#!/usr/bin/env bash

# backs up a specific single directory by making an archive of it (tar.gz).
# the storage location for the created archive is also specified.

target_asset_path="$1"; shift
archive_storage_path="$1"; shift
archive_tag="$1"; shift

if [ -z "$target_asset_path" -o -z "$archive_storage_path" ]; then
  echo Backups up a directory by creating a compressed archive of it in a storage
  echo location.
  echo Requires the path to the folder that will be backed up as the first parameter
  echo and the path to the archive storage directory as the second parameter.
  exit 1
fi

# use a default archive tag if there was none provided.
if [ -z "$archive_tag" ]; then
  archive_tag=folder
fi

sep='_'

tar -czf "${archive_storage_path}/${archive_tag}_bkup_$(date +"%Y$sep%m$sep%d$sep%H%M$sep%S" | tr -d '/\n/').tar.gz" "$target_asset_path" &>>$TMP/$(sanitized_username).scripts.backup_arbitrary.log


