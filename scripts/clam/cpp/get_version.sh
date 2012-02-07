#!/bin/bash
COMPILER=$1
COMPILER_ROOT_DIR=$2
if [ "$COMPILER" = "GNU_LINUX" \
    -o "$COMPILER" = "GNU_DARWIN" ]; then
  # compiler version report for the gnu compiler on linux and darwin.

  # older code is needed for some versions of gcc / suse.
  ver_found=$(gcc -### 2>&1 | grep "gcc version" | sed -e 's/^gcc version \([0-9.][0-9.]*\) .*$/\1/')

#  if [ ! -d "/usr/include/c++/$ver_found" ]; then
#      -a ! -d "/usr/include/c++/$ver_found" ]; then

#    # newest code takes only first two version numbers, since that's how
#    # suse 11.0 at least is listing the includes.
#    ver_found=$(gcc -### 2>&1 | grep "gcc version" | sed -e 's/^gcc version \([0-9.][0-9.]*\) .*$/\1/' | sed -e 's/\([0-9][0-9]*\)\.\([0-9][0-9]*\).*/\1.\2/')

#  fi

  echo "$ver_found"

elif [ "$COMPILER" = "GNU_ARM_LINUX" ]; then
  # compiler version report for the gnu compiler on the arm processor.
  gcc -### 2>&1 | grep "gcc version" | sed -e 's/^gcc version \([0-9.][0-9.]*\) .*$/\1/' 
elif [ "$COMPILER" = "VISUAL_CPP" ]; then
  # compiler version report for ms visual studio.
  ver_raw=`$COMPILER_ROOT_DIR/bin/cl 2>&1 | head -1 | sed -e 's/.*Version \([0-9][0-9]*\)\..*$/\1/'`
  if [ "$ver_raw" = "12" ]; then echo 6;
  elif [ "$ver_raw" = "13" ]; then echo 7;
  elif [ "$ver_raw" = "14" ]; then echo 8;
  elif [ "$ver_raw" = "15" ]; then echo 9;
  elif [ "$ver_raw" = "16" ]; then echo 10;
  fi
else
  echo "0"
fi
