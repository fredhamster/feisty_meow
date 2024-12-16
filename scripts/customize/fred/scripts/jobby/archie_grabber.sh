#!/usr/bin/env bash

# grabs a set of archives from a set of machines.

#hmmm: not tuned for re-use very much yet.
# but this idea could be used for our home machines too... given some good parameter management.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ -z "$ARCHIVE_DIR_PREFIX" ]; then
  # the archive directories will be known by their odd naming, which starts with the below.
  # but we respect if people want to override that default archive directory name.
  ARCHIVE_DIR_PREFIX="z_arch"
fi

# looks for archive directories within a DNS domain for a set of hosts.
# if any archive dirs are found, they are copied to the local host and
# then moved out of the way on the remote host.
function grab_archies()
{
  local domain_piece="$1"; shift
  local host_list="$1"; shift
  for host in $host_list; do
    mkdir -p ${host}.${domain_piece}
    pushd ${host}.${domain_piece}
    local cp_outfile="$(mktemp /tmp/archie_grabber.XXXXXX)"
    netcp ${host}.${domain_piece}:${ARCHIVE_DIR_PREFIX}* . &> "$cp_outfile"
    retval=$?
    if [ $retval -ne 0 ]; then
      cat "$cp_outfile"
      rm "$cp_outfile"
      echo "got return value $retval from copying ${ARCHIVE_DIR_PREFIX}* from ${host}.${domain_piece}; skipping it."
      popd 
      continue
    fi
    rm "$cp_outfile"

    # code below cleans up any archive dirs on the host by hiding them in an
    # old junk folder.  the junk folder can be cleaned up later as desired.
    # the impact is that the archives will only be backed up once, and then
    # moved out of the way before the next run.
    host_strider $DATA_GRAVE_SHUFFLE_COMMAND ${domain_piece} ${host}

    popd 
  done
}

################

# active part of the script, where we go out to a bunch of machines
# to grab the archive folders.

# we'll store the copied archives here.
#hmmm: should make that directory selectable...
mkdir -p $HOME/grabbing_archies
pushd $HOME/grabbing_archies

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

################

# these hosts are all in the ITS domain...

domain="its.virginia.edu"
hostlist="idpprod01 idpprod02 idpprod03 idpprod04 idpprod05 "
grab_archies "$domain" "$hostlist"
hostlist="idpdev01 idpdev02 "
grab_archies "$domain" "$hostlist"
hostlist="idptest01 idptest02 "
grab_archies "$domain" "$hostlist"
hostlist="idpsistest01 idpsistest02 "
grab_archies "$domain" "$hostlist"

hostlist="test-shibboleth-sp02 "
grab_archies "$domain" "$hostlist"

hostlist="tower "
grab_archies "$domain" "$hostlist"

################

# these hosts are in the storage domain...

domain="storage.virginia.edu"
hostlist="admin03 admin-hsz02-s admin-lab nasman02-s "
grab_archies "$domain" "$hostlist"

################

popd


