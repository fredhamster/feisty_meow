#!/bin/bash

# processes cgi request and passes it on to the real script.

cmd1=$(echo $QUERY_STRING | sed -e "s/.*cmd1=\([^&]*\).*$/\1/" | sed -e "s/\+/ /g" )
cmd2=$(echo $QUERY_STRING | sed -e "s/.*cmd2=\([^&]*\).*$/\1/" | sed -e "s/\+/ /g" )

echo "Content-type: text/plain"
echo ""
echo ""
#echo "query string is $QUERY_STRING"
#echo "cmd1=$cmd1"
#echo "cmd2=$cmd2"
bash /home/www-data/feisty_meow/scripts/buildor/version_utils.sh $cmd1 $cmd2

