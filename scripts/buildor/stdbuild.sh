#!/bin/bash
# stdbuild: a build script that applies to most unix make based builds
# for things that you trust enough to install.  this does the configure
# phase, then makes the project, then installs it.
#
# go to the main folder of the project you're building before you run this.

source $SHELLDIR/core/date_stringer.sh

echo "$(date_stringer)"
echo "Building application from $(\pwd)"
echo "    via standard 'configure;make;sudo make install' process..."
echo ""

echo "Running configure script..."
./configure
if [ $? != 0 ]; then
  echo "Something went wrong during 'configure'."
  exit 1
fi

echo "Calling make..."
make
if [ $? != 0 ]; then
  echo "Something went wrong during 'make'."
  exit 1
fi

echo "About to install application as root..."
echo "sudo password needed to install from $(\pwd):"
sudo make install
if [ $? != 0 ]; then
  echo "Saw a failure to su or to install application."
fi

echo "Finished building in $(\pwd)."
echo "$(date_stringer)"

