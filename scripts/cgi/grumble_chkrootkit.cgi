#!/usr/bin/env bash

# checks the rootkit report file to see if anything is listed.

export CHKFILE=$TMP/test_for_rootkit_entries.txt
export QUIET_REPORT=/var/www/rootkit_report_quiet.txt
export LOUD_REPORT=/var/www/rootkit_report_expert.txt

# cgi text starting thing...
#not right yet
echo
echo

if [ ! -f $QUIET_REPORT ]; then
  echo No file found for $QUIET_REPORT
  exit 3
fi

grep "LKM Trojan" $QUIET_REPORT >$CHKFILE

if [ -z "$(cat $CHKFILE)" ]; then
  echo found no content in the quiet report, skipping complaint.
  rm $CHKFILE

  exit 8
fi

echo found some content in the quiet report, looking up bad processes.

if [ ! -f $LOUD_REPORT ]; then
  echo No file found for $LOUD_REPORT
  exit 9
fi

grep "PID.*not in readdir output" $LOUD_REPORT >$CHKFILE

echo after the pid grep
  
if [ -z "$(cat $CHKFILE)" ]; then
  echo "didn't find our expected phrase."
  exit 23
fi

export BAD_PROCESS=$(sed -e "s/^.*PID *\([0-9][0-9]*\).*$/\1/" <$CHKFILE)
if [ -z "$BAD_PROCESS" ]; then
  echo "didn't find our expected phrase."
  exit 32
fi
echo bad process number is $BAD_PROCESS

echo bad proc command line is:
export CMDLINE=$(cat /proc/$BAD_PROCESS/cmdline)
echo $CMDLINE

if [ $BAD_PROCESS -ne 1 ]; then

  if [ ! -z "$(echo $CMDLINE | grep ini)" ]; then

    echo "How come the process $BAD_PROCESS is called \"$CMDLINE\""
    echo "when the init process is always supposed to be process 1?"

  fi

fi



