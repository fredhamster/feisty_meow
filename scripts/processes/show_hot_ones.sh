#!/bin/bash

# shows the current processes ordered by cpu usage.

# generate a nice separator.
for ((i=0; i<14; i++)); do line+='='; done

echo $line
echo "Process list by CPU usage for $(date)"
ps wuax --sort -%cpu | head -n 8
echo $line
