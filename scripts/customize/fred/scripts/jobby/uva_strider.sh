#!/usr/bin/env bash

# iterates across the set of machines we use in UVa ITS all the time and
# performs a set of actions per host.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

################

# active part of the script, where we go out to a bunch of machines to get things done.

################

init_op="$1"; shift
remote_op="$1"; shift
cleanup_op="$1"; shift

#hmmm: fix the docs here.  basename of app, e.g.
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

# these hosts are all in the "its" domain...

domain="its.virginia.edu"
hostlist="idpprod01 idpprod02 idpprod03 idpprod04 idpprod05 "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idpdev01 idpdev02 "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idptest01 idptest02 "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"
hostlist="idpsistest01 idpsistest02 "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

hostlist="test-shibboleth-sp02 "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

hostlist="tower "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

################

# these hosts are in the "storage" domain...

domain="storage.virginia.edu"
hostlist="admin03 admin-hsz02-s admin-lab nasman02-s "
remote_activater "$init_op" "$remote_op" "$cleanup_op" "$domain" "$hostlist"

################

