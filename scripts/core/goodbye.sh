#!/bin/bash
bash $SHELLDIR/byemessage.sh

nohup bash $SHELLDIR/byejob.sh >/dev/null 2>&1

if [ -f $TMP/trash.last_keep_awake_process ]; then
  kill -9 $(cat $TMP/trash.last_keep_awake_process)
  \rm $TMP/trash.last_keep_awake_process
fi

\exit 0
