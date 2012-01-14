#!/bin/bash
echo hostname is $(hostname) for make_display >trash.trash
case $DISPLAY in
	unix*) echo $(hostname):0.0 ;;
	*) echo $DISPLAY
	echo setting to $DISPLAY >>trash.trash ;;
esac
