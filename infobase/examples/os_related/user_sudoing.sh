
# if this script is run as sudo, then at some point it may be useful to become another
# user, in order to run something from within their context.  this is one way to do it
# with a semi-interactive set of steps...
sudo -u chronical bash <<eof
echo hello this is \$(sanitized_username)
echo we be in \$(pwd)
echo "where you at?"
eof

# this can also be done by running a script with all those commands in it.
sudo -u chronical bash /home/chronical/thing_to_run_as_chronical.sh

