#!/bin/bash
# writes an uptime report to a file in the home directory which is named
# after the current machine's hostname.
export hosty=$(hostname | sed -e 's/^\([^.]*\).*$/\1/')
export REPORT_FILE="$HOME/${hosty}_uptime.log"
echo "$($BINDIR/mdate) -- $(uptime)" >>"$REPORT_FILE" 2>&1 
