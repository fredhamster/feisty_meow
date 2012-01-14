#!/bin/bash
# clean out temporary files that might make clam think the state is other
# than a fresh build next time.
rm -f $FLAG_FILES $SUB_FLAG_FILES
if [ ! -z "$CLAM_ERROR_SOUND" ]; then
  bash $CLAM_DIR/sound_play.sh $CLAM_ERROR_SOUND
fi
exit 1;  # cause an error to stop the make.
