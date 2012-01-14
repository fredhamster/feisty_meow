#!/bin/bash

if [ $# -lt 2 ]; then
  echo "This program needs two parameters.  The first is the directory where spam"
  echo "emails are stored.  The second parameter is a text file containing non-spammy"
  echo "email addresses (that is, a whitelist of good email addresses--people who are"
  echo "allowed to send you email).  The script will scan that directory for any email"
  echo "addresses in the whitelist file and report files that might potentially not be"
  echo "spam (i.e., false positives)."
  exit 2323
fi

if [ ! -d "$1" ]; then
  echo "This program needs a spam email *directory* as its first parameter."
  exit 2324
fi

if [ ! -f "$2" ]; then
  echo "This program needs a whitelist *file* as its second parameter."
  exit 2325
fi

export SCAN_DIR="$1"  # where we will scan spam emails.
export ADDRESS_FILE="$2"  # where our addresses live.

# a generated script that reports when a matching pattern was found in a file.
export ITERATE_COMMAND="$(mktemp "$TMP/zz_saitercmd.XXXXXX")"
# was in there "$2" but not defined?
echo '\
  matched=; \
  if [ ! -z "$(head -50 "$1" | grep "From:" | grep -f "$ADDRESS_FILE" -i )" ]; then \
    matched="$1"; \
  fi; \
  if [ ! -z "$matched" ]; then \
    echo "found non-spam email address in : $matched"; \
  fi \
' >$ITERATE_COMMAND

find $SCAN_DIR -type f -exec bash $ITERATE_COMMAND "{}" ';' 

rm -f $ITERATE_COMMAND 

