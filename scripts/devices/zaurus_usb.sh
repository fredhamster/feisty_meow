#!/bin/bash
modprobe usb-uhci
ifconfig eth1 42.56.70.1 netmask 255.255.255.255 up
route add -host 42.56.70.2 eth1 

