#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"  # for check_result.

function build_xsede()
{
  pushd ~/xsede/code/cak0l/trunk
  check_result "entering trunk directory"
  echo "Build starting at: $(date)"
  \rm -rf unit-test-reports
  ant -Dbuild.targetArch=64 build
  check_result "building the trunk"
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
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  check_result "cleaning the trunk"
  popd
  build_xsede
}

