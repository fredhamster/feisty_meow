#!/usr/bin/env bash

export OUTF=$HOME/email_headers.txt

for i in $(find . -type f); do 
  echo "==== $i ====" >>$OUTF
  grep "^From " $i >>$OUTF
  echo >>$OUTF
done


