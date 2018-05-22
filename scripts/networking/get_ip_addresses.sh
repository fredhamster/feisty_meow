ifconfig | grep "inet " | sed -e "s/^.*inet \([0-9.]*\) .*$/\1/"
