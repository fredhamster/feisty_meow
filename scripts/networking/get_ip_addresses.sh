ifconfig | grep "inet addr:" | sed -e "s/^.*addr:\([0-9.]*\) .*$/\1/"
