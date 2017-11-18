
# snippet of code to set all the temp folders and genesis2 state dir on a stable local
# temporary directory.  do not use /localtmp if it will be deleted!  this stuff is
# expected to persist until the user decides to clean things up.

# use a local temporary directory if possible.
if [ -d /localtmp ]; then
  export FAST_LOCAL_STORAGE=/localtmp/$USER
  export TMP=$FAST_LOCAL_STORAGE/tempo
  mkdir -p $TMP &>/dev/null
  chmod -R 700 $FAST_LOCAL_STORAGE

  # plan on putting the state directory onto there.
  export GENII_USER_DIR=$FAST_LOCAL_STORAGE/state-dir
fi

# after the above, load feisty meow scripts and they will take advantage of the
# TMP folder we set above.

