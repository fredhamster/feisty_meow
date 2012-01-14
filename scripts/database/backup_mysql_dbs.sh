#!/bin/bash
source $SHELLDIR/core/date_stringer.sh

temphost=$(hostname | sed -e 's/\([^.]*\)\..*/\1/')
tar -czf /z/stuffing/archives/backup_dbs_${temphost}_$(date_stringer).tar.gz /home/archives/mysql_dbs/*
