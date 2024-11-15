#!/bin/bash

# grabs a set of archives from a set of machines.
# not tuned for re-use very much yet.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# the archive directories will be known by their odd naming, which starts with the below.
ARCHIVE_DIR_PREFIX="z_arch"

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
#hmmm: could display output on error.
    rm "$cp_outfile"
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from copying ${ARCHIVE_DIR_PREFIX}* from ${host}.${domain_piece}; skipping it."
      popd 
      continue
    fi
    # the tricky code below just cleans up any archive dirs on the host by hiding them
    # under an old junk folder.  that can be cleaned up later as desired.
    ssh ${host}.${domain_piece} bash <<EOF
{ \
DATA_GRAVE="\$(mktemp -d \$HOME/old_junk.XXXXXX)"; \
mkdir -p \$DATA_GRAVE; \
echo "moving old $ARCHIVE_DIR_PREFIX* folders into \$DATA_GRAVE"; \
mv $ARCHIVE_DIR_PREFIX* \$DATA_GRAVE; \
}
EOF
    popd 
  done
}

####

# active part of the script, where we go out to a bunch of machines
# to grab the archive folders.

# we'll store the copied archives here.
mkdir -p $HOME/grabbing_archies
pushd $HOME/grabbing_archies

####

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

####

# these hosts are in the storage domain...

domain="storage.virginia.edu"
hostlist="admin03 admin-hsz02-s admin-lab nasman02-s "
grab_archies "$domain" "$hostlist"

####

popd


