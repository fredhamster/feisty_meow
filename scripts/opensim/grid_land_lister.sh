#!/usr/bin/env bash

# creates a file with the information about land on the
# simulator.

#hmmm: need to abstract the gridroot user, the output file name
#(although support a default), and the database name.

mysql -u gridroot -p > $HOME/dump_of_opensim_land_table.txt <<eof
use wildmutt_opensim;
select RegionUUID,Name,Description,OwnerUUID from land;
eof

