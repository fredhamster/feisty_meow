#!/bin/bash
if [ -f "$BUILD_LIST_FILE" ]; then
  echo Compiling Objects [$(cat $BUILD_LIST_FILE | while read input_text; do echo $(basename $input_text); done )]
  rm -f $(cat $BUILD_WHACK_FILE)
#echo got line to run: $*
  eval "${@}"
  rm -f $BUILD_LIST_FILE $BUILD_WHACK_FILE
  if [ $? -ne 0 ]; then
    . "$CLAM_DIR/exit_make.sh"
  fi
fi

