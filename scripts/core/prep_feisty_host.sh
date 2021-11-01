#!/bin/bash

# this is the feisty meow host preparation script.  it installs all the packages required to run and build feisty meow scripts and applications.
# this script may still be a bit incomplete; we definitely use a lot of unix and linux tools in different scripts.

# preconditions and dependencies--this script itself depends on:
#   feisty meow
#   bash
#   anything else?

####

# something borrowed...
function exit_on_error() {
  if [ $? -ne 0 ]; then
    echo -e "\n\nan important action failed and this script will stop:\n\n$*\n\n*** Exiting script..."
    error_sound
    exit 1
  fi
}

####

# load feisty meow environment here, but first test that we *can* load it.

#hmmm: currently, this script needs the system to have already been configured?
#  that's the implication of calling launch_feisty...
#  can we find that same bootstrapping code that will reconfigure first?
#more about this...
#    hmmm: we need clean starty type approach!  must not expect feisty to already be configured for use!
#    e.g.?? $ bash /opt/feistymeow.org/feisty_meow/scripts/core/reconfigure_feisty_meow.sh
#    hmmm: above ALSO ESSENTIAL TO GET RIGHT!

PHASE_MESSAGE="Checking integrity of Feisty Meow subsystem"
if [ -z $FEISTY_MEOW_APEX ]; then
  false; exit_on_error $PHASE_MESSAGE
fi

# simple brute force check.  can we go there?
pushd $FEISTY_MEOW_APEX &> /dev/null
exit_on_error locating feisty meow top-level folder
popd &> /dev/null

# now ask feisty if it's there; should work as long as our scripts are in place.
bash $FEISTY_MEOW_APEX/scripts/core/is_feisty_up.sh
exit_on_error $PHASE_MESSAGE

# standard load-up.
#hmmm: this will currently fail if reconfigure has never been called.
source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

####

# first the crucial bits for scripts to work...

PHASE_MESSAGE="installing perl file and diff modules"

# ubuntu or debian or other apt-based OSes...
if whichable apt; then
  sudo apt install libfile-which-perl libtext-diff-perl
  exit_on_error $PHASE_MESSAGE
# rpm based with yum available...
elif whichable yum; then  
  sudo yum install perl-File-Which perl-Text-Diff
  exit_on_error $PHASE_MESSAGE
# macos based...
elif [ ! -z "$IS_DARWIN" ]; then

#hmmm: not quite right yet...
  brew install blah blah? lots?
  exit_on_error $PHASE_MESSAGE

# windows-based with cygwin (or we'll fail out).
elif [ "$OS" == "Windows_NT" ]; then
 
#hmmm: install apt-cyg!
# we need this to do the following step, so why not automate that?
# can we at least check for the packages we absolutely need?

#hmmm: can we bootstrap and still survive on the basic cygwin modules if already installed?
#  then we could use our huge list to get the rest!

#hmmm: is there any other way to get the missing ones, that we need for apt-cyg?

  apt-cyg install perl-File-Which perl-Text-Diff
  exit_on_error $PHASE_MESSAGE
fi

####

# then the builder packages...

PHASE_MESSAGE="installing code builder packages"

# ubuntu or debian or other apt-based OSes...
if whichable apt; then
  sudo apt install build-essential librtmp-dev libcurl4-gnutls-dev libssl-dev
  exit_on_error $PHASE_MESSAGE
# rpm based with yum available...
elif whichable yum; then  
  sudo yum install gcc gcc-c++ openssl-devel.x86_64 curl-devel
  exit_on_error $PHASE_MESSAGE
# macos based...
elif [ ! -z "$IS_DARWIN" ]; then

#hmmm: not quite right yet...
  brew install blork blork? lots?
  exit_on_error $PHASE_MESSAGE

# windows-based with cygwin (or we'll fail out).
elif [ "$OS" == "Windows_NT" ]; then
 
#hmmm: unknown list needed still...
  apt-cyg install fugazi combustinatorinibasil scampnific
  exit_on_error $PHASE_MESSAGE
fi

####

# install other external packages and whatnot.

#hmmm: anything else to get installed?
  #hmmm: java?
  #hmmm: python?
  #hmmm: perl itself!?


####

# get ready to finish up.

#...finishing steps...  if any.

# all done now.
exit 0

####


#############################
#scav line
#############################

The "kona" collection depends on Java version 8 or better.
| Ubuntu:
| Set up the java PPA archive as described here:
| https://launchpad.net/~webupd8team/+archive/ubuntu/java

#not needed at the moment.
#echo "bailing because script is immature.  farts!"
#exit 1

