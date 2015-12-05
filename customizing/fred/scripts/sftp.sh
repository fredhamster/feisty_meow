#!/bin/bash

source "$FEISTY_MEOW_LOADING_DOCK/custom/scripts/pick_credentials.sh"

# a wrapper for the file transfers using secure shell.
\sftp -i "$keyfile" $*

