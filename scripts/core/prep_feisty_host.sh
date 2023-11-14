#!/usr/bin/env bash

# this is the feisty meow host preparation script.  it installs all the packages required to run and build feisty meow scripts and applications.

# preconditions and dependencies--this script itself depends on:
#   feisty meow
#   bash
#   anything else?

# note that this list of items is never totally complete, since feisty meow keeps expanding and mutating.  for example, we now have a few
# python scripts starting to sneak in.  there are assuredly lots of python packages we should be installing in here now, but we aren't yet.
# this is a best effort script, to at least get feisty meow able to run its core scripts and to build.  although it's always appreciated
# when things we rely on get installed too...

####

ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
CORE_SCRIPTS_DIR="$(echo "$ORIGINATING_FOLDER" | tr '\\\\' '/' )"
THIS_TOOL_NAME="$(basename "$0")"

# set up the feisty_meow dir.
pushd "$CORE_SCRIPTS_DIR/../.." &>/dev/null
#source "$CORE_SCRIPTS_DIR/functions.sh"
echo originating folder is $ORIGINATING_FOLDER
export FEISTY_MEOW_APEX="$(/bin/pwd)"
echo feisty now apex is FEISTY_MEOW_APEX=$FEISTY_MEOW_APEX

# establish whether this is darwin (MacOS) or not.
export IS_DARWIN="$(echo $OSTYPE | grep -i darwin)"

####

# helper scripts...
#
# these come from other places in the feisty meow ecosystem, but they are here
# because this script is kind of a bootstrapping process for our scripts and code.
# we don't want to assume anything about the presence of the rest of feisty meow
# at this point in the process.

function exit_on_error() {
  if [ $? -ne 0 ]; then
    echo -e "\n\nan important action failed and this script will stop:\n\n$*\n\n*** Exiting script..."
#    error_sound
    exit 1
  fi
}

####

function whichable()
{
  local to_find="$1";
  shift;
  local WHICHER="$(/usr/bin/which which 2>/dev/null)";
  if [ $? -ne 0 ]; then
      echo;
      return 2;
  fi;
  local sporkenz;
  sporkenz=$($WHICHER "$to_find" 2>/dev/null);
  local err=$?;
  echo $sporkenz;
  return $err
}

####

#hmmm: copy to mainline scripts.
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

#hmmm: copy to mainline scripts also.
# figures out what kind of installation packager application is
# used on this machine and invokes it to install the list of
# packages provided as parameters.
function install_system_package()
{
  local packages=("${@}")
    # pull out the array of packages from the command line.
  if [ ! -z "$IS_DARWIN" ]; then
    # macos based...
echo "installing for darwin"
    if ! whichable brew; then
      echo "Could not locate the brew installation system."
      echo "Please install brew, which is required for MacOS feisty meow installs."
      return 1
    fi
    brew install "${packages[@]}"
    return $?
  elif whichable apt; then
    # ubuntu or debian or other apt-based OSes...
echo "installing for apt"
    sudo apt install "${packages[@]}"
    return $?
  elif whichable yum; then  
    # rpm based with yum available...
echo "installing for yum"
    sudo yum install "${packages[@]}"
    return $?
  elif [ "$OS" == "Windows_NT" ]; then
    # windows-based with cygwin (or we'll fail out currently).
echo "installing for apt-cyg"
    if apt_cyg_finder; then
      apt-cyg install perl-File-Which perl-Text-Diff
      return $?
    else
      echo "apt-cyg is not currently available on this system.  please install cygwin and apt-cyg."
      return 1
    fi
  else
    echo "Unknown installer application for this platform."
    return 1
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

BASE_PHASE_MESSAGE="Feisty Meow subsystems integrity check: "

# is our main variable set?
PHASE_MESSAGE="$BASE_PHASE_MESSAGE presence of FEISTY_MEOW_APEX variable"
if [ -z "$FEISTY_MEOW_APEX" ]; then
  false; exit_on_error $PHASE_MESSAGE
fi

# simple brute force check.  can we go there?
PHASE_MESSAGE="$BASE_PHASE_MESSAGE check on directory $FEISTY_MEOW_APEX"
pushd $FEISTY_MEOW_APEX &> /dev/null
exit_on_error $PHASE_MESSAGE
popd &> /dev/null

# now ask feisty if it's there; should work as long as our scripts are in place.
#PHASE_MESSAGE="$BASE_PHASE_MESSAGE inquiry is_feisty_up"
#bash $FEISTY_MEOW_APEX/scripts/core/is_feisty_up.sh
#exit_on_error $PHASE_MESSAGE

# standard load-up.
#hmmm: this will currently fail if reconfigure has never been called.
#NO NO NO. source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
# we are preparing to get feisty running; how can we use feisty during
# that process?  so bad.

####

# figure out which kind of OS we're on first, from ground up by seeing
# how this system can install things.

opsystem_here=unknown

if [ ! -z "$IS_DARWIN" ]; then
  # macos based...
  opsystem_here=macos
elif whichable apt; then
  # ubuntu or debian or other apt-based OSes...
  opsystem_here=debianesque
elif whichable yum; then  
  # rpm based with yum available...
  opsystem_here=redhatty
elif [ "$OS" == "Windows_NT" ]; then
  # windows-based with cygwin (or we'll fail out currently).
  opsystem_here=windoze
fi
echo decided OS is $opsystem_here

####

# default value of our package list is to fail out, since we
# may not be able to determine what OS this is running on.
PAX=(noop)

####

# first we install the low-level crucial bits for scripts to work...

PHASE_MESSAGE="installing script modules"

if [ "$opsystem_here" == "debianesque" ]; then
  PAX=(libfile-which-perl libtext-diff-perl)
elif [ "$opsystem_here" == "redhatty" ]; then
  PAX=(perl-Env perl-File-Which perl-Text-Diff)
elif [ "$opsystem_here" == "macos" ]; then
  PAX=(dos2unix openssl)
elif [ "$opsystem_here" == "windoze" ]; then
  PAX=(perl-File-Which perl-Text-Diff)
fi

install_system_package "${PAX[@]}"
exit_on_error $PHASE_MESSAGE

####

# then the builder packages...

PHASE_MESSAGE="installing code builder packages"

if [ "$opsystem_here" == "debianesque" ]; then
  PAX=(mawk build-essential librtmp-dev libcurl4-gnutls-dev libssl-dev)
elif [ "$opsystem_here" == "redhatty" ]; then
  PAX=(curl-devel gcc gcc-c++ make gawk openssl-devel.x86_64 zlib-devel)
elif [ "$opsystem_here" == "macos" ]; then
  PAX=(mawk gpg meld openjdk)
elif [ "$opsystem_here" == "windoze" ]; then
  PAX=(gawk libcurl-devel meld mingw64-i686-openssl openssl openssl-devel libssl-devel zlib-devel)
fi

install_system_package "${PAX[@]}"
exit_on_error $PHASE_MESSAGE

####

# install other external packages and whatnot.

PHASE_MESSAGE="installing additional helper packages"

if [ "$opsystem_here" == "debianesque" ]; then
  PAX=(screen python3 python3-pip xserver-xorg xorg-docs)
elif [ "$opsystem_here" == "redhatty" ]; then
  PAX=(screen python3 python3-pip xserver-xorg xorg-docs)
elif [ "$opsystem_here" == "macos" ]; then
  PAX=(screen python3 xquartz xorg-docs)
elif [ "$opsystem_here" == "windoze" ]; then
  PAX=(screen python3 python3-pip xserver-xorg xorg-docs)
fi

install_system_package "${PAX[@]}"
exit_on_error $PHASE_MESSAGE

####

# get ready to finish up.

#...finishing steps...  if any.

# all done now.
exit 0

####


#############################
#scavenging line
#############################

#The "kona" collection depends on Java version 8 or better.
#| Ubuntu:
#| Set up the java PPA archive as described here:
#| https://launchpad.net/~webupd8team/+archive/ubuntu/java

#not needed at the moment.
#echo "bailing because script is immature.  farts!"
#exit 1

