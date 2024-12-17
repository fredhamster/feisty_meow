#!/usr/bin/env bash

# implements an approach for striding a set of hosts with a local init
# method, a remote action method, and a local clean-up method.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# documentation...
function print_instructions()
{
  echo "
$(basename $0 .sh): runs a remote action on a set of hosts.

This script needs at least four parameters:

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
4) The hostname to run the script on, or a domain name when used with a set
of hosts.
5) The set of hosts, if a domain name is provided.  Otherwise, there are only
four parameters.  Note that if this parameter is provided, it may contain
multiple hosts, but they must be included in this single parameter.  See the
examples below for more info.

Examples:

$(basename $0 .sh) \$HOME/my_init.sh \$HOME/my_remote_actor.sh \$HOME/my_cleaner.sh singlehost.mydomain.org
   # invokes the remote script on a single host.

$(basename $0 .sh) \$HOME/my_init.sh \$HOME/my_remote_actor.sh \$HOME/my_cleaner.sh mydomain.org \"thathost1 thathost2 thathost3\"
   # invokes the remote script on multiple hosts.
"
}

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
    squelch_unless_error bash "$init_op" "${host}.${domain_piece}"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from initialization script '$init_op' for ${host}.${domain_piece}; skipping it."
      continue
    fi

    # now we make the remote call by relying on the host strider.
    echo "invoking remote action operation '$remote_op'..."
    squelch_unless_error bash $FEISTY_MEOW_SCRIPTS/core/host_strider.sh "${remote_op}" "${domain_piece}" "${host}"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "got return value $retval from remote action script '$remote_op' on ${host}.${domain_piece}; skipping it."
      continue
    fi

    # then invoke the clean-up call to get things right again on the local host.
    echo "invoking local clean-up operation '$cleanup_op'..."
    squelch_unless_error bash "$cleanup_op" "${host}.${domain_piece}"
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
domain_piece="$1"; shift
host_piece="$1"; shift

if [ -z "$init_op" -o -z "$remote_op" -o -z "$cleanup_op" -o -z "$domain_piece" ]; then
  print_instructions
  exit 1
fi

################

# if we made it to here, let's try doing their action on all those hosts they provided...
instigate_remote_calls "$init_op" "$remote_op" "$cleanup_op" "$domain_piece" "$host_piece"

################

