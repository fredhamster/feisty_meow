#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/pick_credentials.sh"

# a wrapper for the file transfers using secure shell.
\sftp -i "$keyfile" $*

