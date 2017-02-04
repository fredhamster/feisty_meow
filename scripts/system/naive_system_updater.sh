
# load feisty meow aliases.
source $HOME/feisty_meow/scripts/core/launch_feisty_meow.sh

sudo apt-get update -y
check_result "problem while doing 'apt-get update'"
sudo apt-get dist-upgrade -y
check_result "problem while doing 'apt-get dist-upgrade'"


