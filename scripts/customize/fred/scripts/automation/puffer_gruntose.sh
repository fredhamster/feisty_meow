#!/usr/bin/env bash

# strides across all gruntose hosts and updates their codes.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

host_strider $FEISTY_MEOW_SCRIPTS/rev_control/puffer.sh $GRUNTOSE_DOMAIN $GRUNTOSE_HOSTLIST &> ~/puffer_output_$(date_stringer).log


