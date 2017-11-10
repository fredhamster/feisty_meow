#!/bin/bash

# this script works on a specialized type of git checkout that has been configured
# to push to a "downstream" repository, while still pulling from its normal origin.
# the downstream destination might be a github or sourceforge repository that is
# loaded from a personal repository or server.
#
# it is assumed that you have already added your ssh key to your github account.
#
# to set up the repository for relaying downstream, just do the normal checkout
# or clone on it from the real origin.  for example:
#
# $ git clone git://feistymeow.org/feisty_meow feisty_relay
#
# change into that new directory:
#
# $ pushd feisty_relay
#
# and then add the downstream remote repository:
#
# # github example of add:
# $ git remote add downstream git@github.com:fredhamster/feisty_meow.git
#
# # sourceforge example of add:
# $ git remote add downstream ssh://fred_t_hamster@git.code.sf.net/p/feistymeow/trunk
#
# once the repository has been created, you can synch all updates that
# have been checked into the origin repository with the downstream version
# by running this command:
#
# push_repo_downstream ~/relay_repo_folder

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null
test_or_die "changing to directory: $dir"

#switch this to the perform revctrl with a do_git_careful_update



# get everything from the origin.
git fetch origin
test_or_die "running git fetch origin"


#these are all shots in the dark.
# use our careful updater!

git remote update

# get everything from the origin.
git fetch --all

# merge the changes in.
git merge --no-ff --all

# turn off occasionally troublesome setting before checkin.
unset GIT_SSH

# send the little boat down the stream to the dependent repository.
git push downstream master
test_or_die "running the git push downstream"

popd &>/dev/null

