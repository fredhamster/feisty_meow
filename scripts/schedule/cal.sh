#!/usr/bin/env bash
# cal: a nicer interface to the unix cal program.

function our_date()
{
  date '+%A %B %d %Y'
}

case $# in
	0)	set $(our_date); m=$2; y=$4 ;;	# no arguments; use today
	1)	m=$1; set $(our_date); y=$4 ;;	# 1 argument; use this year
	*)	m=$1; y=$2 ;;			# 2 arguments; month and year
esac

#echo would run, after first case: "/usr/bin/cal $m $y"

case $m in
	jan*|Jan*|JAN*)	m=1 ;;
	feb*|Feb*|FEB*)	m=2 ;;
	mar*|Mar*|MAR*)	m=3 ;;
	apr*|Apr*|APR*)	m=4 ;;
	may*|May*|MAY*)	m=5 ;;
	jun*|Jun*|JUN*)	m=6 ;;
	jul*|Jul*|JUL*)	m=7 ;;
	aug*|Aug*|AUG*)	m=8 ;;
	sep*|Sep*|SEP*)	m=9 ;;
	oct*|Oct*|OCT*)	m=10 ;;
	nov*|Nov*|NOV*)	m=11 ;;
	dec*|Dec*|DEC*)	m=12 ;;
	[1-9]|10|11|12) ;;				# numeric month
	*)			y=$m; m="" ;;		# just a year
esac

#echo running: "/usr/bin/cal $m $y"
/usr/bin/cal $m $y				# run the real one
