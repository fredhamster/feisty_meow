#!/bin/bash
# this script acquires our local IP address and connects over to zooty
# to write a status file there.  this enables us to later connect backwards
# to our home system without being hosed by the floating IP address.

#hmmm:  none of the user info below will work for others: parameterize it.

server=zooty.koeritz.com
username=fred
local_user=fred
tempdir=/tmp  # where we generate our files.

source $HOME/feisty_meow/scripts/core/launch_feisty_meow.sh

soundfile=$FEISTY_MEOW_DIR/database/sounds/woouoo.wav
if [ ! -z "$1" ]; then
  soundfile=$1
fi

ip_file="$(mktemp ${tempdir}/ip_${USER}_$(hostname | sed -e "s/\..*$//").XXXXXX)"

# get live ip address
pushd $tempdir
wget http://automation.whatismyip.com/n09230945.asp -O "$ip_file" 

chmod 644 "$ip_file"
my_ip=$(head "$ip_file")

echo "my ip is [$my_ip]"

# send the file over to the server.
# note that the local_user here is expected to have a certificate that
# gets us access to the server.  this needs to be on the local machine
# for sftp to run without a login prompt.
sudo -u $local_user sftp $username@$server <<eof
if [ ! -d gen ]; then
  mkdir gen
fi
cd gen
put $ip_file $(hostname | sed -e "s/\..*$//")_ip.txt
eof

\rm -f "$ip_file"

popd

