#!/bin/bash

# check for whether we see a .git folder.
if [ ! -d ".git" ]; then
  echo This script needs to run in the directory where a git repository lives,
  echo but we do not see a .git directory here.
  exit 1
fi

# makes git checkouts not be as intensive on the server.
git config --global pack.windowMemory "100m"
git config --global pack.SizeLimit "100m" 
git config --global pack.threads "1"


