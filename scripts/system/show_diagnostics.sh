#!/usr/bin/env bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

echo '=============='
echo $(date_stringer | sed -e 's/_/ /g' | sed -e 's/\([0-9][0-9]\) \([0-9][0-9]\)$/:\1:\2/')
echo '=============='
echo
echo "free memory:"
echo
free
echo
echo '=============='
echo
echo "Simple iostat:"
echo
iostat
echo
echo '=============='
echo
echo "Simple mpstat:"
echo
mpstat
echo
echo '=============='
echo
echo "Simple vmstat:"
echo
vmstat
echo
echo '=============='
echo
echo "full process list:"
echo
ps wuax
echo
echo '=============='
echo

