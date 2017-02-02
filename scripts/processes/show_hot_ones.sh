#!/bin/bash

# shows the current processes ordered by cpu usage.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

sep
echo "Process list by CPU usage for $(date)"
ps wuax --sort -%cpu | head -n 8
sep
