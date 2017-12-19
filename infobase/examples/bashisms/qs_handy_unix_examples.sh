#!/bin/bash

#
# these great examples of handy unix tidbits were donated by "q. black".
#

# list a directory tree
ls -R | grep ":$" | sed -e 's/:$//' -e 's/[^-][^\/]*\//--/g' -e 's/^/   /' -e 's/-/|/'

# list directory sizes (biggest first)
du -h $(du -s * |sort -nr|awk '{print $2}')

# this will sort a date field of the form: DD-MON-YYYY HH:MM:SS
sort +0.7 -1 +0.3M -0.6 +0 -0.2 +1

# this will sort a date field of the form: MON DD HH:MM:SS YYYY
sort +3 -4 +0M +1n

# this will sort a date field of the form: MON DD HH:MM:SS
sort +0M +1n
 
# this will sort a date field of the form: Date: Tue Feb  3 09:17:58 EST 2004
sort +6 -7 +2M +3n +4

# display all lines from a certain line onward
start_line=132
|awk "{if (NR >= ${start_line}){print \$0}}"

# display all lines after a token
sed '1,/CUT HERE/d'

# print the first and last lines
sed -n '1,1p;$,$p'

# signal bash about a window size change
kill -winch $$

# show the date 1 year, 2 months and 3 days ago
date -v -1y -v -2m -v -3d

# set the date back 1 year
sudo date $(date -v -1y +%Y%m%d%H%M)

# output the standard date format for setting the time
# get the date
date -u +%Y%m%d%H%M.%S
# set the date
date -u (cut and paste from above)

# convert one date format to another (output is in the current time zone)
old_date="Aug 27 15:24:33 2005 GMT"
new_date=$(date -j -f "%b %e %T %Y %Z" "${old_date}" +%D)
echo ${new_date}
# returns "08/27/05"

# output the modification time of a file in different format
file=
date -j -f "%b %e %T %Y" "$(ls -lT ${file} |awk '{print $6,$7,$8,$9}')"

# output the number of days until a certain date
target_date="Sep  2 15:20:20 2005 GMT"
target_seconds=$(date -j -f "%b %e %T %Y" +%s "${target_date}" 2>/dev/null)
diff_seconds=$(expr ${target_seconds} - $(date +%s))
diff_days=$(expr ${diff_seconds} / 86400)
echo "${diff_days} day(s)"

# these commands can be used to fill in missing times in a "uniq -c" count
# of times.
# output 24 hours in one minute increments
for h in $(jot -w %02d - 0 23 1); do
    for m in $(jot -w %02d - 0 59 1); do
	echo "   0 ${h}:${m}"
    done
done
# sort them together, and remove any 0 counts if an count already exists
sort +1 +0rn out1 out2 |uniq -f 1

# output with w3m to get basic html word wrap
w3m -T "text/html" -dump -cols 72 <<EOF
    <p>
    This test verifies basic networking and that the ${product_short_name}
    can reach it's default gateway.
EOF

# another way to format text for output
fmt 72 <<EOF
This test verifies basic networking and that the ${product_short_name}
can reach it's default gateway.
EOF

# smtpcrypt "printf"
{
char *jkwbuf = NULL;
asprintf(&jkwbuf, "JKW: msg->used = %ld\n", msg->used);
BIO_write(sc->log, jkwbuf, strlen(jkwbuf)+1);
free(jkwbuf);
}

# rolling diff of a list of files (diff a & b, then b & c,...)
last=
for i in $(ls -1rt); do
    if [ ! -z "${last}" ]; then
	diff -u ${last} ${i}
    fi
    last=${i}
done

# clearing and restoring chflags
file=
old_chflags=$(ls -lo ${file}|awk '{print $5}')
chflags 0 ${file}
# do whatever
if [ ."${old_chflags}" != ."-" ]; then
    chflags ${old_chflags} ${file}
fi

# way to do standard edits to files
file=
{
    # append line(s) after a line, "i" to insert before
    echo '/www_recovery/a'
    echo 'mithril ALL = (root) NOPASSWD: /usr/local/libexec/destroyer'
    echo '.'
    # modify a line
    echo 'g/^xntpd_program=/s,^xntpd_program=.*$,xntpd_program="ntpd",'
    # delete a line
    echo 'g/^controls key secret =/d'
    echo 'x!'
} | ex - ${file}

# how to search for errors in the last 24 hours
# note that this command does not work quite right.  The sort is off early
# in the year because the dates do not have the year.
# Also sed never sees the /CUT HERE/ when it is the first line.
(echo "$(date -v-24H "+%b %e %H:%M:%S") --CUT HERE--"; \
    zgrep -h "cookie" /var/log/messages*)|sort +0M| \
    sed '1,/CUT HERE/d'
# This version fixes those problems.  It adds the file year to the date
# and puts a marker at the start of the list.
(echo "$(date -j -f "%s" 0 "+%Y %b %e %H:%M:%S") --ALWAYS FIRST--"; \
    echo "$(date -v-24H "+%Y %b %e %H:%M:%S") --CUT HERE--"; \
    for i in /var/log/messages*; do
	year=$(ls -lT ${i}|awk '{print $9}')
	zgrep -h "cookie" ${i}|while read line; do
	    echo "${year} ${line}"
	done
    done)|sort +0n +1M| sed '1,/CUT HERE/d'

# process a list of quoted values
{
    # It tends to be easiest to use a 'here-document' to feed in the list.
    # I prefer to have the list at the start instead of the end
    cat <<EOF
	'general' 'network node0' 'private address'
	'general' 'options node2' 'kern securelevel'
EOF
}| while read line; do
    eval set -- ${line}
    config=$1; shift
    section=$1; shift
    key=$1; shift

    echo "confutil value \"${config}\" \"${section}\" \"${key}\""
done

# Method to read lines with a "for" loop, without spawning a subshell
NEWLINE='
'
OIFS="${IFS}"
IFS="${NEWLINE}"
for line in $(cat /etc/passwd | sort -r); do
    IFS="${OIFS}"

    # do whatever you want here
    echo "line = ${line}"

    IFS="${NEWLINE}"
done
IFS="${OIFS}"

# generate a histogram of characters in a file
cat file|
    awk '{for (i=1; i <= length($0); i++) {printf("%s\n",substr($0,i,1))}}'|
    sort|uniq -c

# show line lengths for a file
cat file| awk '{print length($0)}'| sort -n




# get the modification time of a directory or file and then reset the time.
target=/usr/local/etc/pkdb
save_date=$(ls -ldT ${target}|awk '{print $6,$7,$8,$9}')
save_date=$(date -j -f "%b %e %T %Y" "${save_date}" +"%Y%m%d%H%M.%S")
# later
touch -t ${save_date} ${target}


# detect NULL bytes in a file
file=
hexdump -e '"%_u\n"' ${file}|grep -q '^nul$'
if [ $? -eq 0 ]; then
else
fi


# calculate average
cd /tmp
uudecode
begin 644 bc.average
M<V-A;&4],PIT;W1A;#TP"F-O=6YT/3`*=VAI;&4@*#$I('L*("`@(&YU;2`]
M(')E860H*0H@("`@:68@*&YU;2`]/2`P*2!B<F5A:SL*("`@(&-O=6YT*RL*
M("`@('1O=&%L("L](&YU;0I]"B)T;W1A;"`]("([('1O=&%L"B)C;W5N="`]
K("([(&-O=6YT"B)A=F5R86=E(#T@(CL@=&]T86P@+R!C;W5N=`IQ=6ET"@``
`
end
(cat data; echo "0") |bc -q bc.average


