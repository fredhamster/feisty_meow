#!/usr/bin/env bash

# grabs a set of archives from the set of ITS machines.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ -z "$ARCHIVE_DIR_PREFIX" ]; then
  # the archive directories will be known by their odd naming, which starts with the below.
  # but we respect if people want to override that default archive directory name.
  ARCHIVE_DIR_PREFIX="z_arch"
fi

################

# go out to a bunch of ITS machines to grab the archive folders.

# we'll store the copied archives here.
#hmmm: should make that directory selectable...
export COPY_TARGET_TOP="$HOME/grabbing_archies"
mkdir -p "$COPY_TARGET_TOP"

export ARCHIVE_SNAGGER_COMMAND="$(mktemp "$TMP/archive_snagger.sh.XXXXXX")"
echo '\
#!/usr/bin/env bash
# copies the archives we find in the remote home for the user which start with the expected prefix.
hostname="$1"; shift
ARCHIVE_DIR_PREFIX="'$ARCHIVE_DIR_PREFIX'"
pushd "$COPY_TARGET_TOP"
mkdir -p "${hostname}"
pushd "${hostname}"
cp_outfile="$(mktemp /tmp/archie_grabber_copying.XXXXXX)"
netcp ${hostname}:${ARCHIVE_DIR_PREFIX}* . &> "$cp_outfile"
retval=$?
if [ $retval -ne 0 ]; then
  cat "$cp_outfile"
fi
rm "$cp_outfile"
popd
popd
exit $retval
' > $ARCHIVE_SNAGGER_COMMAND

# write a script that we'll run remotely to clean up after we get a copy of the archives.
export DATA_GRAVE_SHUFFLE_COMMAND="$(mktemp "$TMP/data_engraver.sh.XXXXXX")"
echo '\
#!/usr/bin/env bash
# moves the newly copied archives into a junk folder.
ARCHIVE_DIR_PREFIX="'$ARCHIVE_DIR_PREFIX'"
DATA_GRAVE="$(mktemp -d $HOME/old_junk.XXXXXX)"
mkdir -p $DATA_GRAVE
cd  # jump to normal top of home.
echo "moving old $ARCHIVE_DIR_PREFIX* folders into $DATA_GRAVE"
mv $ARCHIVE_DIR_PREFIX* $DATA_GRAVE
' > $DATA_GRAVE_SHUFFLE_COMMAND

export LOCAL_CLEANER_COMMAND="$(mktemp "$TMP/post_copy_local_cleaner.sh.XXXXXX")"
echo '\
#!/usr/bin/env bash
# the last step is to clean up anything for this transfer that we want to dump.
hostname="$1"; shift
ARCHIVE_DIR_PREFIX="'$ARCHIVE_DIR_PREFIX'"
echo "no steps for cleanup yet..."
' > $LOCAL_CLEANER_COMMAND

################

# do our thing with the uva strider to get any archives...
uva strider "$ARCHIVE_SNAGGER_COMMAND" "$DATA_GRAVE_SHUFFLE_COMMAND" "$LOCAL_CLEANER_COMMAND"

################

# clean-up for our own script here...
rm "$ARCHIVE_SNAGGER_COMMAND" "$DATA_GRAVE_SHUFFLE_COMMAND" "$LOCAL_CLEANER_COMMAND"

################

