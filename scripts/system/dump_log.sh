#!/usr/bin/env bash

# dumps out the log files according to the provided pattern, but makes sure that
# each one is dumped in chronological order, and any compressed logs are unpacked
# first.

function assemble_log_file()
{
  logpath="$1"; shift

  # build an array of all the file names, in reverse order since we want the oldest
  # files listed first.
  full_set=($(ls -1 -r "$logpath"*))
  if [ ${#full_set[*]} -lt 1 ]; then
    echo "No log files were found matching the pattern '$full_set'"
    exit 1
  fi

  logdump="$(mktemp /tmp/$(sanitized_username)_logdump.XXXXXX)"

  for logy in ${full_set[*]}; do
#echo logy is $logy
    if [[ $logy =~ .*\.gz ]]; then
      gzip -d -c "$logy" >>"$logdump"
    else
      cat "$logy" >>"$logdump"
    fi
  done

  cat "$logdump"
  \rm -f "$logdump"
}


##############

logpath="$1"; shift

if [ -z "$logpath" ]; then
  echo "$(basename $0 .sh): Log file dumper"
  echo
  echo "This script requires a log path, which should be the prefix of some log files"
  echo "that it will dump out.  All files matching the prefix are printed to standard"
  echo "output, and the log entries will be printed in chronological order.  Any"
  echo "compressed log files will be unpacked first before printing."
  echo
  echo "Example:"
  echo -e "\t$(basename $0 .sh) /var/log/syslog"
  echo
  exit 1
fi

assemble_log_file "$logpath"


