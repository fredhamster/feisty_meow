#!/usr/bin/env bash

# just create an archive dir for testing our grabber.

NONSENSE_SCRIPT="$HOME/testingly_tingler.txt"

function date_stringer() { local sep="$1"; shift; if [ -z "$sep" ]; then sep='_'; fi; date +"%Y$sep%m$sep%d$sep%H%M$sep%S" | tr -d '/\n/'; }

cat "$0" > "$NONSENSE_SCRIPT"
export archdir="z_archies_${RANDOM}"; mkdir -p "$archdir"; cp "$NONSENSE_SCRIPT" "$archdir/blarghy_script_$(date_stringer).txt"
rm "$NONSENSE_SCRIPT"

