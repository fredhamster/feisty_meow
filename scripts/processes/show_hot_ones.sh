#!/bin/bash

# shows the current processes ordered by cpu usage.

sep
echo "Process list by CPU usage for $(date)"
ps wuax --sort -%cpu | head -n 8
sep
