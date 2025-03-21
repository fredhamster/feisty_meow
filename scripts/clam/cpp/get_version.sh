#!/usr/bin/env bash
COMPILER=$1
COMPILER_ROOT_DIR=$2
if [ "$COMPILER" = "GNU_LINUX" \
    -o "$COMPILER" = "GNU_DARWIN" \
    -o "$COMPILER" = "GNU_WINDOWS" ]; then
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
else
  echo "0"
fi
