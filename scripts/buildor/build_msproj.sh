#!/bin/bash
project_name="$1"; shift
if [ -z "$project_name" ]; then
  echo This script requires an ms project file name to build.
  exit 3
fi

devenv.com $project_name -Build Release
