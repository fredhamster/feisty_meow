#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

zip -r $HOME/gffs_backup_$(date_stringer).zip $GENII_INSTALL_DIR --exclude "*/.svn/*" --exclude "*/bin.eclipse/*" --exclude "*/bin.ant/*" --exclude "*/codegen/*" --exclude "*/genned-src/*" --exclude "*/genned-obj/*" &>$TMP/zz_codebackup.log
