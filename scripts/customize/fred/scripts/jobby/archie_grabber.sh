#!/bin/bash

# grabs a set of archives from a set of machines.
# not tuned for re-use very much yet.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function grab_archies()
{
  local domain_piece="$1"; shift
  local host_list="$1"; shift
  for host in $host_list; do
    mkdir -p ${host}.${domain_piece}
    pushd ${host}.${domain_piece}
    netcp ${host}.${domain_piece}:z_arch* . 
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "Error $retval returned from copying z_arch* from ${host}.${domain_piece}"
      popd 
      continue
    fi
    ssh ${host}.${domain_piece} '{ \
echo hello; \
echo "howdy ho!"; \
echo more stuff here.; \
}'
    popd 
  done
}

mkdir -p $HOME/grabbing_archies
pushd $HOME/grabbing_archies

domain="its.virginia.edu"
#hostlist="idpprod01 idpprod02 idpprod03 idpprod04"
#grab_archies "$domain" "$hostlist"
hostlist="idpdev01 idpdev02"
grab_archies "$domain" "$hostlist"
#hostlist="idptest01 idptest02"
#grab_archies "$domain" "$hostlist"
#hostlist="idpsistest01 idpsistest02"
#grab_archies "$domain" "$hostlist"

#domain="storage.virginia.edu"
#hostlist="manage-s admin02 admin-hsz-s"
#grab_archies "$domain" "$hostlist"

popd


