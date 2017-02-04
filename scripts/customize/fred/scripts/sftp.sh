#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_LOADING_DOCK/custom/scripts/pick_credentials.sh"

# a wrapper for the file transfers using secure shell.
\sftp -i "$keyfile" $*

