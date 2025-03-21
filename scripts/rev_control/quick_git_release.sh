#!/usr/bin/env bash

# a simple and quick method for making a new release, merging it into the main branch,
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
# ready to merge in to the main branch as a new release.  the process
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
$scriptname: this script requires a version name to use for the new branch
name.  this version name will become the new tag on the repository.
"
    return 1
  fi

  # jump into the top of the feisty meow codebase.
  pushd $FEISTY_MEOW_APEX
  # make up a release name based on the version number.
  local new_release="release-${new_version}"

  echo "About to create the release called '$new_release' as a branch"
  echo "on the git repository -- please hit Enter."
  read line

  # make sure we're working on the dev branch, since that's where our releases come from.
  git checkout dev
  exit_on_error checking out the dev branch

  # inflate all the git branches we might need, getting all their latest.
  rpuffer
  exit_on_error running rpuffer on the dev branch to update it

  # make a new branch for the release based on the dev branch.
  git checkout -b $new_release dev
  exit_on_error checking out a new branch called $new_release

  # bump feisty meow version. 
  bash ./scripts/generator/next_version.sh
  exit_on_error bumping version for feisty meow codebase

  # check in the changes in the new release branch, which now includes a revised version.
#echo about to commit--hit enter
#read line
  git commit -a
  exit_on_error committing all changes

  # not sure if we really need to check in the release branch as a remote, but we like to see it in the list.
#echo about to push new release branch--hit enter
#read line
  git push --set-upstream origin "$new_release"

  # grab out the main branch as the active one.
#echo about to check out main--hit enter
#read line
  git checkout main
  exit_on_error checking out main branch

  rpuffer
  exit_on_error running rpuffer on main branch to update it

  # merge the main branch with the new release.
#echo about to merge--hit enter
#read line
  git merge --no-ff $new_release
  exit_on_error merging in the new release in main

  # let the committer see the most recent changes.
  echo
  echo "=> launching gitk to show you the full set of changes;"
  echo "=> please prepare an excellent commit comment."
  gitk
  continue_on_error launching gitk

  # now make a tag for the new release, which is where we should go crazy with the detailed
  # and useful comments for what has changed in this release, gathered from the gitk that
  # we just launched.  this should include all of the work on the development branch since
  # the last release...
#echo about to TAG--hit enter
#read line
  git tag -a $new_version
  exit_on_error tagging new version as $new_version

  # commit the full set of changes for the main branch now, including the tags.
#echo about to commit main branch with all those changes--hit enter
#read line
  rcheckin .
  exit_on_error checking in the changes in main branch

  # switch back to the dev branch.
#echo switching to dev branch--hit enter
#read line
  git checkout dev
  exit_on_error checking the dev branch out again

  # merge in the latest changes from main, which should only be the revised version really.
#echo merging in from release branch to dev--hit enter
#read line
  git merge --no-ff $new_release
  exit_on_error merging the release changes into the dev branch
#echo pushing merged dev branch up

  # now update anything from our merged state in remote.
  git push 
  exit_on_error pushing merged dev branch up

  # done with the serious actions.
  echo -e "\ncompleted the release of version $new_version\n"

  # back to where we started.
  popd
}

make_new_feisty_meow_release "$1"

