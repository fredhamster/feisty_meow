#!/usr/bin/env bash

# gets rid of any customized resolv.conf file for the local machine and reinstates the standard link for it.

sudo rm /etc/resolv.conf
sudo ln -s /var/run/resolvconf/resolv.conf /etc/resolv.conf
sudo resolvconf -u
