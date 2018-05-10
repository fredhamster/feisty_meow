#!/bin/bash

# screeno: a simple alias for putting something in the background where its logs can be
# tended by the screen utility.

app="$1"; shift
title="$1"; shift
if [ -z "$app" ]; then
#print_instructions
  echo This utility needs an application name to launch into the background.
  echo The app will be managed via the screen utility so its logs are available
  echo later, plus it can be interacted with directly.
  exit 1
fi
if [ -z "$title" ]; then
  title="$app"
#hmmm: we should be sanitizing that guy!  don't want a long deal title.
fi

screen -L -S "$title" -d -m "$app"




