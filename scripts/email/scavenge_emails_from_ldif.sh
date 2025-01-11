#!/usr/bin/env bash
if [ $# -lt 1 ]; then
  echo "$(basename $0) usage: the first parameter should be an"
  echo "LDIF file that this script will extract email addresses from."
  exit 1
fi
if [ ! -f "$1" ]; then
  echo "$(basename $0): the file $1 does not seem to exist"
  exit 1
fi
grep -i mail: "$1" | sed -e "s/^.*[Mm][aA][Ii][Ll]: *\(.*\)$/\1/" | tr A-Z a-z | sort | uniq | sort 
