#!/usr/bin/env bash

# main port
sudo ufw allow 9000

#also needed?
#sudo ufw allow 8002

# open ports for all the regions.
i=10008; while [ $i -le 10022 ]; do echo $i ; sudo ufw allow $i; ((i++)); done

# get the new rules loaded up and restart the firewall.
sudo ufw reload
sudo ufw disable
sudo ufw enable


