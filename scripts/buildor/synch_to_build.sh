#!/bin/bash

if [ $# -lt 1 ]; then
  echo "The first parameter must be the directory to synch into."
  exit 1
fi
instdir="$1"

chmod -R u+w "$instdir"

exval=0

for curr_file in "$instdir"/*.dll; do 
  base=$(basename "$curr_file")
  if [ -f "$FEISTY_MEOW_DIR/dll/$base" ]; then
    cp -f "$FEISTY_MEOW_DIR/dll/$base" "$curr_file"
    if [ $? -ne 0 ]; then
      echo "** Error copying $base to $instdir"
      exval=1
    fi
  fi
done
for curr_file in "$instdir"/*.exe; do 
  base=$(basename "$curr_file")
  if [ -f "$FEISTY_MEOW_DIR/exe/$base" ]; then
    cp -f "$FEISTY_MEOW_DIR/exe/$base" "$curr_file"
    if [ $? -ne 0 ]; then
      echo "** Error copying $base to $instdir"
      exval=1
    fi
  fi
done

exit $exval

