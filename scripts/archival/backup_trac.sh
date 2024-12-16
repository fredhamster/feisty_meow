#!/usr/bin/env bash

# backs up a trac repository into a tar.gz file.

export THISDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

trac_path="$1"; shift
archive_path="$1"; shift

# call our arbitrary backer upper, since this is a simple single directory case.
bash $THISDIR/backup_arbitrary.sh "$trac_path" "$archive_path" "trac_bkup"


