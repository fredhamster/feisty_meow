
source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sudo apt-get update -y
check_result "problem while doing 'apt-get update'"
# new magic to tell dpkg to go with existing config files.  let's see if it works!
sudo apt-get dist-upgrade -y -o Dpkg::Options::="--force-confold"
check_result "problem while doing 'apt-get dist-upgrade'"


