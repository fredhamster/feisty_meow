
source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sudo apt-get update -y
test_or_die "problem while doing 'apt-get update'"
# newest magic to tell dpkg to go with existing config files and force non-interactive version.
export DEBIAN_FRONTEND=noninteractive
sudo apt-get dist-upgrade -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold"
test_or_die "problem while doing 'apt-get dist-upgrade'"


