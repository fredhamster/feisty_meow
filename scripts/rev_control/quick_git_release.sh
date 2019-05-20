#!/bin/bash

# a simple and quick method for making a new release, merging it into the master branch,
# and tagging it with a new tag for the release.
# currently needs to be told the new release name, which is actually also gotten from
# the "next_version" script.  if these differ, there will be confusion for users about
# the right version number.  for now, look at the config file in production to get
# the current version, add one to the revision number, and pass major.minor.revision to
# this release script.

source $FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh

#hmmm: fixes--
#  done + need to make the version a parameter passed to here, and complain if it's not provided.
#  + better if we could get the version automatically, but there is a circular dependency where we want to check out the new branch by version number.

# these are the steps i take on feisty meow when i have a dev branch that is
# ready to merge in to the master branch as a new release.  the process
# includes adding the tag for the new release and such.  there are manual
# steps for adding the commit comments, including an introspection phase
# with gitk before the release commit comment is created.
function make_new_feisty_meow_release()
{
  # snag the version number from the passed parameter.
  local new_version="$1"; shift
  local scriptname="$(basename $0 .sh)"

  if [ -z "$new_version" ]; then
    echo "\
$scriptname: this script requires a version number to use for the
branch name and release tag name of the new release.
"
    return 1
  fi

  # jump into the top of the feisty meow codebase.
  pushd $FEISTY_MEOW_APEX
  # make up a release name based on the version number.
  local new_release="release-${new_version}"
  # make a new branch for the release based on the dev branch.
echo about to git checkout
read line
  git checkout -b $new_release dev
  exit_on_error checking out a new branch called $new_release
  # bump feisty meow version. 
  bash ./scripts/generator/next_version.sh
  exit_on_error bumping version for feisty meow codebase
echo about to commit
read line
  # check in the changes in the new release branch, which now includes a revised version.
  git commit -a
  exit_on_error committing all changes
echo about to check out master
read line
  # grab out the master branch as the active one.
  git checkout master
  exit_on_error checking out master branch
echo about to merge
read line
  # merge the master branch with the new release.
  git merge --no-ff $new_release
  exit_on_error merging in the new release in master
  # let the committer see the most recent changes.
  echo "=> launching gitk to show you the full set of changes;"
  echo "=> please prepare a kick-ass commit comment."
  gitk
  exit_on_error launching gitk
  # now make a tag for the new release, which is where we should go crazy with the detailed
  # and useful comments for what has changed in this release, gathered from the gitk that
  # we just launched.  this should include all of the work on the development branch since
  # the last release...
echo about to TAG
read line
  git tag -a $new_version
  exit_on_error tagging new version as $new_version
echo about to commit master branch with all those changes
read line
  # commit the full set of changes for the master branch now, including the tags.
  rcheckin .
  exit_on_error checking in the changes in master branch
echo switching to dev branch
read line
  # switch back to the dev branch.
  git checkout dev
  exit_on_error checking the dev branch out again
echo merging in from master
read line
  # merge in the latest changes from master, which should only be the revised version really.
  git merge --no-ff $new_release
  exit_on_error merging the release changes into the dev branch
  # back to where we started.
  popd
}


make_new_feisty_meow_release "$1"


