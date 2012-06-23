#!/bin/bash

function build_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant -Dbuild.targetArch=64 build
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
  popd
}

function rebuild_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  popd
  build_cak0l
}

