#!/usr/bin/env bash

###############################################################################
#                                                                             #
#  Name   : redo_nvidia_drivers                                               #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2004                                                #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    This shell script can re-build the NVIDIA graphics drivers for Linux     #
#  after a new kernel has been installed, given a few prerequisites:          #
#                                                                             #
#      1) You are logged in as root.                                          #
#      2) You are logged in at a console, and not under X.  (Use the key      #
#         combo Ctrl-Alt-F1 to get to the first console.)                     #
#      3) The home directory for root has a subdirectory called "drivers".    #
#      4) The drivers directory contains at least one NVIDIA graphics driver  #
#         that they offer in the form of NVIDIA-Linux-x86-1.0-6111-pkg1.run   #
#         or some similar hideous name.                                       #
#                                                                             #
#  General Note:                                                              #
#    After this script is successfully run, it will leave the system at the   #
#    X windows login screen.  Note that the root user will still be logged    #
#    in back on the console where the script was executed.  You should go     #
#    back to the console again and log out before considering the system to   #
#    be secure again.                                                         #
#                                                                             #
#  Note for SuSE users:                                                       #
#    After you run this script successfully, then you should not need to      #
#    run SaX to set up your display and video card.  They should already      #
#    be correct from your previous settings.  If, however, this is the        #
#    first time you've ever installed the driver, then you may have to        #
#    run SaX and fiddle with settings to get them right.  The key in SaX      #
#    is to get the best match for your monitor, to ensure that your video     #
#    driver is the appropriate choice for your video card, and to ensure      #
#    that SaX says it will use the 'nvidia' driver (not the 'nv' driver).     #
#                                                                             #
###############################################################################
#  This script is free software; you can redistribute it and/or modify it     #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy  #
#  of the License online.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

# This script requires an up to date kernel source tree in /usr/src/linux,
# at least for SuSE 9.1 to work.  You, as root, could make a symbolic link
# for that if it's missing:
#   ln -s /usr/src/linux-2.6.x.y.z /usr/src/linux
# where the x.y.z is your current 2.4 or 2.6 kernel.

# get the current run level so we'll be in the same state afterwards.
export RUNLEV=$(runlevel | sed -e 's/.* \(.*\)$/\1/' )
#echo "The current run-level is \"$RUNLEV\"."

export DRIVER_DIR=$HOME/drivers/nvidia
  # this is assuming a directory under root's home directory called "drivers"
  # where the NVIDIA drivers are located in a subdirectory called "nvidia".
  # this directory must exist and it must have at least one NVIDIA-Linux.*.run
  # file in it.
if [ ! -d "$DRIVER_DIR" ]; then
  echo "The driver directory $DRIVER_DIR does not exist.  Either set that variable"
  echo "in this script or create the directory and ensure that the NVIDIA driver"
  echo "in the form "NVIDIA-Linux*.run" is present in it."
  exit 43
fi

export KERNEL_VER=$(kernelversion)

echo "The current kernel version is $KERNEL_VER."

if [ "$KERNEL_VER" = "2.6" ]; then
  echo "Going to linux source code directory..."
  cd /usr/src/linux

  echo "Cloning the source tree configuration..."
  make cloneconfig >"$(mktemp "$TMP/zz_cloneconfig.XXXXXX")"

  echo "Preparing the source tree..."
  make prepare >"$(mktemp $TMP/zz_prepare_all.XXXXXX)"
fi

echo "Going to root's drivers directory..."
cd $DRIVER_DIR

if [ "$RUNLEV" != "3" ]; then
  # X windows might still be running.  let's go to single user mode.
  echo "Ensuring that X-windows is shut down..."
  init 3
  sleep 7  # give it a little time to close things.
else
  echo "X-windows is already shut down."
fi

# this script supports the NVIDIA-Linux-x86.XXXX.run version of the nvidia
# driver.  the ".run" file is just a shell script that we can easily
# execute from here.

# replace SOME_NVIDIA_PACKAGE definition below if you don't want it to
# pick the first one it finds in the drivers directory.  i tend to only have
# the most current one in there anyway, so this works and i don't need to
# keep updating the batch file.  
export SOME_NVIDIA_PACKAGE=$(find . -name "NVIDIA-Linux*.run" | head -1)
if [ -z "$SOME_NVIDIA_PACKAGE" ]; then
  echo "No NVIDIA packages in the form NVIDIA-Linux*.run were found."
  echo "Please download the latest NVIDIA graphics driver from the NVIDIA"
  echo "web page: http://www.nvidia.com"
  exit 20
fi

echo "Using the NVIDIA package called: $SOME_NVIDIA_PACKAGE"

# we need to add an extra bit to the command if this is 2.6.
export SRC_PATH_EXTRA=
if [ "$KERNEL_VER" = "2.6" ]; then
  export SRC_PATH_EXTRA='--kernel-source-path /usr/src/linux'
fi

if ! bash $SOME_NVIDIA_PACKAGE $SRC_PATH_EXTRA ; then
  echo "command was: bash $SOME_NVIDIA_PACKAGE $SRC_PATH_EXTRA"
  echo ""
  echo "Did you cancel the installation?  If not, then it failed."
  exit 12
fi

if [ "$RUNLEV" != "3" ]; then
  echo "If everything went successfully, then we should be able to start up"
  echo "the X Window System right now.  Let's see..."
  init $RUNLEV
else
  echo "You'll have to start X-windows yourself to test the new driver; we're"
  echo "staying in the previous single-user mode."
fi

