#!/bin/bash

# this script modifies the linux kernel for maximum tcp buffer size, which can
# improve long-haul transfers over a wan.

# new maximum buffer size to set.
new_max=2097152

echo "net.core.wmem_max=$new_max" >> /etc/sysctl.conf
echo "net.core.rmem_max=$new_max" >> /etc/sysctl.conf

echo "net.ipv4.tcp_rmem= 10240 87380 $new_max" >> /etc/sysctl.conf
echo "net.ipv4.tcp_wmem= 10240 87380 $new_max" >> /etc/sysctl.conf

echo "net.ipv4.tcp_window_scaling = 1" >> /etc/sysctl.conf

echo "net.ipv4.tcp_timestamps = 1" >> /etc/sysctl.conf

echo "net.ipv4.tcp_sack = 1" >> /etc/sysctl.conf

echo "net.ipv4.tcp_no_metrics_save = 1" >> /etc/sysctl.conf

echo "net.core.netdev_max_backlog = 5000" >> /etc/sysctl.conf

