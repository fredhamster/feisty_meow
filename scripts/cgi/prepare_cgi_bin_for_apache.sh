#!/bin/bash

# this script needs to run as root.  it will copy all the important
# assets for cgi into the right places.

cd ~/yeti/scripts/cgi
cp call_version_utils.sh cgi_cat.pl cgi_display.pl cgi_nechung.cgi cgi_show_file_date.sh \
    count.cgi nechung.cgi \
  /usr/lib/cgi-bin

cd ~/yeti/scripts/database
cp *show_stripper.sh *movie_seeker.sh *movie_stripper.sh \
  /usr/lib/cgi-bin

cd ~/yeti/scripts/files
cp show_directory_listing.sh \
  /usr/lib/cgi-bin

cd ~/yeti/database
cp fortunes.dat \
  /usr/lib/cgi-bin

cp ~fred/yeti/bin/$(arch)/*nechung* /usr/lib/cgi-bin

chmod a+x /usr/lib/cgi-bin/*

/usr/lib/cgi-bin/nechung /usr/lib/cgi-bin/fortunes.dat

