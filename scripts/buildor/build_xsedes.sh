#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

function build_xsede()
{
  if [ -z "$GENII_INSTALL_DIR" ]; then
    echo GENII_INSTALL_DIR is not set.
    return 1
  fi
  pushd $GENII_INSTALL_DIR
  if [ $? -ne 0 ]; then return 1; fi
  echo "Build starting at: $(date)"
  \rm -rf unit-test-reports
#  # update the libs first.
#  ant update
  if [ $? -ne 0 ]; then return 1; fi
  # then build the trunk.
  ant -Dbuild.targetArch=64 build
  if [ $? -ne 0 ]; then return 1; fi
  # fix memory limits.
#hmmm: clean these up.
  if [ -f runContainer.sh ]; then
    sed -i -e "s/-Xmx512M/-Xmx2G/" "runContainer.sh" 
    chmod 755 "runContainer.sh" 
  fi
  if [ -f grid ]; then
#    sed -i -e "s/-Xmx512M/-Xmx1G/" "grid"
    chmod 755 "grid"
  fi
  if [ -f runContainer.bat ]; then
    sed -i -e "s/-Xmx512M/-Xmx2G/" "runContainer.bat"
    chmod 755 "runContainer.bat" 
  fi
  if [ -f grid.bat ]; then
#    sed -i -e "s/-Xmx512M/-Xmx1G/" "grid.bat"
    chmod 755 "grid.bat"
  fi
  echo "Build done at: $(date)"
  popd
}

function rebuild_xsede()
{
  if [ -z "$GENII_INSTALL_DIR" ]; then
    echo GENII_INSTALL_DIR is not set.
    return 1
  fi
  pushd $GENII_INSTALL_DIR
  ant clean
  if [ $? -ne 0 ]; then return 1; fi
  popd
  build_xsede
}

