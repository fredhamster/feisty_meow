#!/usr/bin/env bash

# iterates across the set of machines we use in UVa ITS all the time and
# performs a set of actions per host.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# takes a triplet of script names and runs them on local and remote hosts...
# first the initialization operation is run locally, then the actual remote
# operation is invoked (remotely), then the clean-up operation is run locally.
# the initialization and clean-up operations are expected to take a hostname,
# and they will each be run for each remote host.
function instigate_remote_calls()
{
  local init_op="$1"; shift
  local remote_op="$1"; shift
  local cleanup_op="$1"; shift
  local domain_piece="$1"; shift
  local host_list="$1"; shift
  # variables used later.
  local retval

  for host in $host_list; do

    # first we call our initialization process.
    echo "invoking local initialization operation '$init_op'..."
    squelch_unless_error bash "$init_op"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from initialization script '$init_op' for ${host}.${domain_piece}; skipping it."
      continue
    fi

    # now we make the remote call by relying on the host strider.
    echo "invoking remote action operation '$remote_op'..."
    squelch_unless_error host_strider "${remote_op}" "${domain_piece}" "${host}"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from remote action script '$remote_op' on ${host}.${domain_piece}; skipping it."
      continue
    fi

    # then invoke the clean-up call to get things right again on the local host.
    echo "invoking local clean-up operation '$cleanup_op'..."
    squelch_unless_error bash "$cleanup_op"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from clean-up script '$cleanup_op' for ${host}.${domain_piece}; skipping it."
      continue
    fi

  done
}

################

# active part of the script, where we go out to a bunch of machines to get things done.

################

init_op="$1"; shift
remote_op="$1"; shift
cleanup_op="$1"; shift

if [ -z "$init_op" -o -z "$remote_op" -o -z "$cleanup_op" ]; then
  echo "$0: runs an action on all of our ITS machines.
This script needs three parameters:
1) The initialization script, which will run locally before each host action.
This script must take at least a single parameter, which is the hostname,
although it does not need to do anything with that if it's not useful for
initialization.
2) The actual remote action script, which will run on the remote hosts.
This script needs to be self-contained enough to handle doing its job on the
remote side with a minimum of setup--only what's already configured on the
remote host will be available to the script.
3) The clean-up script, which will run locally after a successful remote
action.  This script also needs to accept at least a hostname parameter.
"
  exit 1
fi

################

#while testing, we limit the blast zone...
domain="its.virginia.edu"
hostlist="idpdev01 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
echo BAILING
exit 1

# these hosts are all in the ITS domain...

domain="its.virginia.edu"
hostlist="idpprod01 idpprod02 idpprod03 idpprod04 idpprod05 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idpdev01 idpdev02 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idptest01 idptest02 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idpsistest01 idpsistest02 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

hostlist="test-shibboleth-sp02 "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

hostlist="tower "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

################

# these hosts are in the storage domain...

domain="storage.virginia.edu"
hostlist="admin03 admin-hsz02-s admin-lab nasman02-s "
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

################

popd


