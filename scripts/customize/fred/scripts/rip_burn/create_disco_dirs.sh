#!/bin/bash

FROOT=$HOME/disco

for dir in $FROOT $FROOT/burn $FROOT/converted $FROOT/grind $FROOT/rip; do
  # make the currently chosen directory and ignore if already exists.
  mkdir -p $dir
#check_on_error
  # put a sentinel file in the dir to keep it from being deleted.
  echo keepy yo yes $RANDOM >>"$dir/.keep"
#check_on_error
done
