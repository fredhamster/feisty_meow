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

function apt_cyg_finder()
{
  if whichable apt-cyg; then
    return 0  # success.
#hmmm: is that the right syntax for bash?
  else
    echo "
The apt-cyg tool does not seem to be available for cygwin.
Please follow the install instructions at:
    https://github.com/transcode-open/apt-cyg
"
    return 13  # not found.
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

#hmmm: why two phases?

# first the crucial bits for scripts to work...

PHASE_MESSAGE="installing perl file and diff modules"

if whichable apt; then
  # ubuntu or debian or other apt-based OSes...
  sudo apt install libfile-which-perl libtext-diff-perl
  exit_on_error $PHASE_MESSAGE
elif whichable yum; then  
  # rpm based with yum available...
  sudo yum install perl-File-Which perl-Text-Diff
  exit_on_error $PHASE_MESSAGE
elif [ ! -z "$IS_DARWIN" ]; then
  # macos based...
  brew install dos2unix openssl
  exit_on_error $PHASE_MESSAGE
elif [ "$OS" == "Windows_NT" ]; then
  # windows-based with cygwin (or we'll fail out currently).
  if apt_cyg_finder; then
    apt-cyg install perl-File-Which perl-Text-Diff
    exit_on_error $PHASE_MESSAGE
  fi
fi

####

# then the builder packages...

PHASE_MESSAGE="installing code builder packages"

if whichable apt; then
  # ubuntu or debian or other apt-based OSes...
  sudo apt install mawk build-essential librtmp-dev libcurl4-gnutls-dev libssl-dev
  exit_on_error $PHASE_MESSAGE
elif whichable yum; then  
  # rpm based with yum available...
  sudo yum install mawk gcc gcc-c++ openssl-devel.x86_64 curl-devel
  exit_on_error $PHASE_MESSAGE
elif [ ! -z "$IS_DARWIN" ]; then
  # macos based...
#hmmm: still working on these...
  brew install mawk gpg meld openjdk 
  exit_on_error $PHASE_MESSAGE
elif [ "$OS" == "Windows_NT" ]; then
  # windows-based with cygwin (or we'll fail out).

  if apt_cyg_finder; then
echo need to fix apt cyg install list somewhat.
#hmmm: list is in our docs as a separate file for cygwin.
#      plug those packages into here please.
    apt-cyg install gawk libcurl-devel meld mingw64-i686-openssl openssl openssl-devel libssl-devel zlib-devel
    exit_on_error $PHASE_MESSAGE

#extended set.  just add them?
# xorg-server xorg-docs xlaunch 

  fi
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

