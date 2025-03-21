#!/usr/bin/env bash

# so simple but very handy; drops any partitions that we don't care about before showing the df listing.

\df -h "${@}" | grep -v "loop\|udev\|tmpfs"
