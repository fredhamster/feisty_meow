#!/bin/bash

# iterates through the directories or files passed on the command line and does a summary disk usage, but only the
# things measurable in gigabytes, terabytes, or larger are shown.  the smaller items are just omitted (so anything
# measurable within megabytes, kilobytes, and bytes, as decided by the human-readable du command.

for dir in "${@}"; do
  du -sh "$dir"  2>/dev/null | grep -v "^[ 0-9.]*K\|^[ 0-9.]*M\|^[ 0-9.]*B" 
done
