#!/usr/bin/env bash

# iterates across a set of hosts to remotely execute a given bash script on them.
#
# by chris koeritz

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

####

#hmmm: ugly path thing here.
export SSH_APP=/usr/bin/ssh

# uncomment to enable noisy debugging output.
export DEBUG=true

####

function print_instructions()
{
  echo "
$(basename $0 .sh) usage:
This script will execute a bash script on a set of hosts.
The expected parameters to the script are:

  (1) script file name,
  (2) domain name,
  (3-...) host name(s).

If the domain name is passed as empty (e.g. \"\"), then it will be assumed
that the host list is all FQDNs.
If there is no third parameter, then it is assumed that the domain name is
actually a host name to use.

Examples:

  Run on one remote host:
  bash $0 $FEISTY_MEOW_SCRIPTS/core/inventory.sh orpheus.gruntose.blurgh

  Run on a set of remote hosts using a domain:
  bash $0 $FEISTY_MEOW_SCRIPTS/core/inventory.sh gruntose.blurgh mrowrt orpheus surya

"
}

####

# our workhorse scripts below need to be escaped to successfully
# be passed into a bash prompt, whether local or remote.
# also of possible note is the injection of local variables for various
# purposes, where we've unquoted in the middle of quoted strings.

# emits a script that creates a unique directory and echoes the name.
function emit_directory_creator()
{
  local emission='{
    new_dir_name="$(mktemp -d $HOME/temporary_dir.XXXXXX)"
    mkdir -p "$new_dir_name"
    exitval=$?
    echo "$new_dir_name"
    exit $exitval
  }'
  echo "$emission"
}

# this function emits a script that uses an existing directory for storage,
# generates a unique script name based on the directory, and echoes the
# new name.
function emit_get_unique_script_name()
{
  local storage_dir="$1"; shift
  local emission='{
    new_script_name="$(mktemp '$storage_dir'/temporary_script.sh.XXXXXX)"
    exitval=$?
    if [ $exitval -eq 0 ]; then
      echo yep > "$new_script_name"
      echo "$new_script_name"
    fi
    exit $exitval
  }'
  echo "$emission"
}

# outputs a script that runs the user's script, under its new uniquified remote name.
function emit_script_execution_stanzas()
{
  local storage_dir="$1"; shift
  local remote_script_file="$1"; shift

  local emission='{
    storage_dir="'$storage_dir'"
    script_to_run="'$remote_script_file'"

    tempfile="$(mktemp '$storage_dir'/script_output.XXXXXX)"

    pushd $storage_dir &> /dev/null
    #hmmm: pretty ugly quoting below (source side) to get the output to have proper double and single quotes in it.
    delineator_line=$(dd if=/dev/zero bs=42 count=1 2>/dev/null | tr '\''\0'\'' '\''#'\'')
    echo "$delineator_line"
    #echo "output will be stored in \"$tempfile\" also."
    bash "$script_to_run" >"$tempfile" 2>&1 
    exitval=$?
    popd &> /dev/null
    cat "$tempfile"
    echo "$delineator_line"
    exit $exitval
  }'
  echo "$emission"
}

# emits a chunk of additional code that makes sure the user's normal
# environment is invoked before the script is run.
function emit_environment_scavenger_prefix()
{
  # this one doesn't get curly braces around it, since we want it embedded inline same as rest of target script.
  local emission='
#!/usr/bin/env bash
#echo BASH_VERSION is set to: $BASH_VERSION
user_startup_file="$HOME/.profile"
if [ ! -f "$user_startup_file" ]; then user_startup_file="$HOME/.bash_profile"; fi
if [ ! -f "$user_startup_file" ]; then user_startup_file="$HOME/.bashrc"; fi
if [ ! -f "$user_startup_file" ]; then 
#hmmm: quiet the debug here?
  echo "user startup file not found--script had better be environment independent."
else
  # pull their startup code in, so their expectations are met.
  # and hope this does not nuke our session.
#echo sourcing user startup file: ${user_startup_file}
  source "$user_startup_file"
#echo after startup file sourced: ${user_startup_file}
fi

#original script is included below...
'
  echo "$emission"
}

# tears down our temporary storage directory and any results from the script that got stuff over there.
#hmmm: there will have to be a new process to get generated files back to the caller.
function emit_cleanup_processing()
{
  local storage_dir="$1"; shift
  local emission='{
    storage_dir="'$storage_dir'"
    if [ ! -z "$storage_dir" -a -d "$storage_dir" ]; then
      rm -rf "$storage_dir"
      exitval=$?
    else
      echo "failed to remove directory called: $storage_dir"
      exitval=1
    fi
    exit $exitval
  }'
  echo "$emission"
}

####

# bundles up the actions needed to run a script across ssh in bash on a remote
# host.  the expected parameters are:
#   (1) the name of a function that is visible to this function, and
#   (2) the remote host to run on.
#   (3) optional parameters to pass to the named function.
# the function name passed in must emit valid bash code. its output stream
# will be passed over to a bash session on the remote host for execution.
function run_function_remotely()
{
  local action_function="$1"; shift
  local external_host="$1"; shift
  # get right to it and send the function's output to bash across the great divide.
  # the exit status from ssh will be returned.
  $action_function "${@}" | "${SSH_APP}" "${external_host}" bash
  # catch an error in the action function also...
  combine_pipe_returns 1
}

####

function notify_enter_host() { echo -e "\n==> starting on: $*"; }
function notify_exit_host() { echo -e "<== finished on: $*\n"; }

# goes out to the hosts specified and executes a bash script while embedded in an ssh call.
function sozzle_hosts()
{
  local script_file="$1"; shift
  local host_list="$1"; shift

  # declare local vars here ahead of time, since adding a local declaration
  # in front when they're actually being set completely scorches the exit
  # value from the remote sessions.  apparently both local and export have
  # extra machinery that invalidates the exit value afterwards.
  local remote_storage_dir remote_script_name external_host

  for external_host in $host_list; do

    # tricky codes here start ssh sessions and feed in one or more command lines
    # to: get the remote host prepared for the script, to run the script remotely,
    # and then to clean up the script again.

    #separator 14 '#'
    notify_enter_host $external_host
    # create a directory on the remote host and get its name.
    remote_storage_dir="$(run_function_remotely emit_directory_creator "${external_host}" )"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "warning: initial connection and temporary directory creation failed on host '$external_host' with exit code $retval."
      notify_exit_host $external_host
      continue
    fi

    # using our remote directory created earlier, this creates a randomly
    # and uniquely named script file.
    remote_script_name="$(run_function_remotely emit_get_unique_script_name "${external_host}" "$remote_storage_dir" )"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "warning: unique script filename creation failed on host '$external_host' with exit code $retval."
      notify_exit_host $external_host
      continue
    fi

#    echo "info: remote script name on host '$external_host' created as: $remote_script_name"

    # we need to modify their script file a teensy bit, so that there's some prefix code to
    # pull in the user environment first.
    combined_script_file="$(mktemp /tmp/combo_script_file.sh.XXXXXX)"
    emit_environment_scavenger_prefix > "$combined_script_file"
    cat "$script_file" >> "$combined_script_file"
#hmmm: future option--don't add the extra wrapper to get the environment loaded--bare metal, sorta.

    # copy the script over now that we have a stable target.
    squelch_unless_error rsync -avz "$combined_script_file" "${external_host}:${remote_script_name}" 
    retval=$?
    # clean up our temp file.
    rm "$combined_script_file"
    if [ $retval -ne 0 ]; then
      echo "warning: copying script '$script_file' to host '$external_host' failed with exit code $retval.  file name on target was '$remote_script_name'."
      notify_exit_host $external_host
      continue
    fi

    # actual script execution starts now.
    run_function_remotely emit_script_execution_stanzas "${external_host}" "$remote_storage_dir" "$remote_script_name" 
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "warning: executing script '$script_file' failed on host '$external_host' with exit code $retval."
      notify_exit_host $external_host
      continue
    fi
    
    # now time for some post-script cleanup.
    run_function_remotely emit_cleanup_processing "${external_host}" "$remote_storage_dir"
    retval=$?
    if [ $retval -ne 0 ]; then
      echo "warning: cleaning up storage dir '$remote_storage_dir' failed on host '$external_host' with exit code $retval."
      notify_exit_host $external_host
      continue
    fi

    notify_exit_host $external_host
  done

  # drop a final separator line.
  #separator 14 '#'
}

####

# takes a possibly empty domain and a possibly empty host list and
# does what seems rational to emit a list of FQDNs that will work.
function assemble_full_host_list()
{
  domain="$1"; shift
  hostlist="${@}"

  if [ -z "$hostlist" ]; then
    # they only gave us one host, we believe.  so we'll use it.
    hostlist="$domain"
  else
    # assemble the full host names here for passing in to remote method.
    simple_hostlist=($hostlist)
    hostlist=""
    for item in ${simple_hostlist[@]}; do
      if [ ! -z "$domain" ]; then
        hostlist+="${item}.${domain} "
      else
        hostlist+="${item} "
      fi
    done
  #echo "full host list came out as '$hostlist'"
  fi
  echo $hostlist
}

##############

# active part of the script, where we go out to a bunch of machines
# to do our chosen actions...

# we pass the script name first, since that's mandatory.
# then the domain is the first parm after script name.
# and if there are no other host names, then the domain name is taken
# as the actual hostname.
script_file="$1"; shift

# capture remaining parameters for hostlist.
# this should work fine, since the space char (as separator for the list)
# cannot be in a hostname...
host_list="$(assemble_full_host_list "${@}")"
#echo "host list is now '$host_list'"

if [ -z "$script_file" -o -z "$host_list" ]; then
  print_instructions
  exit 1
fi

# gets the script over to each host in the list and runs it over there.
# output is gathered?(see discussion below)
# afterwards, the script and temporary storage directory are cleaned up.
sozzle_hosts "$script_file" "$host_list"

#hmmm: how do useful results get communicated back?  we can capture output but that's not very clever for anything binary or big etc.
# what about offering a return flight option, where result files can be specified?
# AHA, and that also means we should support multiple script files?  ugh.  no!  but we should support multiple files being delivered!
# we can assume that the script wants some parameters, so if we're told any files as parameters, we can pass the (random remote) names 
# in to the script as command line parameters and success and profit.


