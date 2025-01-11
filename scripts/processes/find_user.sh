#!/usr/bin/env bash
if test $# -lt 1; then
  echo $(basename $0): requires a user id for which to search.;
  \exit 1;
fi
tempname="$(mktemp "$TMP/zz_trash.findme.XXXXXX")"
ps wuax | grep "$1[ 	]*[0-9]" | sed -e '
	/sed/d
	/\/bin\/sh.*\/scripts\/find/d
	/ps -uxg/d' >$tempname
# sed command eliminates field ambiguity for STAT field
cat $tempname| sed -e '/[RSD] N/s/[RSD] N/NUN/' |
awk '/^'$1'/ {
	ORS=""
	print "process #" $2, "started", $9 " "
	if ($9 ~ /^[A-Za-z]/) {
		print $10, "with "
	} else {
		print "with", $11 " "
	}
	print $12, $13, $14, $15, $16, $17 "\n"
	}
' | sort -k 1 -t\# -n
/bin/rm $tempname
