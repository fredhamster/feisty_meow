#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

zip -r $HOME/gffs_fred_code_backup_$(date_stringer).zip $HOME/xsede/code/fred/trunk/ --exclude ".svn" --exclude "bin.eclipse" --exclude "bin.ant" --exclude "codegen" --exclude "genned-src" --exclude "genned-obj" &>$TMP/zz_codebackup.log
