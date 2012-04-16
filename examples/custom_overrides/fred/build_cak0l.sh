#!/bin/bash

function build_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant -Dbuild.targetArch=64 build
  # fix memory limits.
#hmmm: clean these up.
  if [ -f runContainer.sh ]; then
    sed -e "s/-Xmx512M/-Xmx2G/" < "runContainer.sh" >"$TMP/${USER}_edited.tmp"
    mv "$TMP/${USER}_edited.tmp" "runContainer.sh" 
    chmod 755 "runContainer.sh" 
  fi
  if [ -f grid ]; then
    sed -e "s/-Xmx512M/-Xmx1G/" < "grid" >"$TMP/${USER}_edited.tmp"
    mv "$TMP/${USER}_edited.tmp" "grid" 
    chmod 755 "grid"
    popd
  fi
  if [ -f runContainer.bat ]; then
    sed -e "s/-Xmx512M/-Xmx2G/" < "runContainer.bat" >"$TMP/${USER}_edited.tmp"
    mv "$TMP/${USER}_edited.tmp" "runContainer.bat" 
    chmod 755 "runContainer.bat" 
  fi
  if [ -f grid.bat ]; then
    sed -e "s/-Xmx512M/-Xmx1G/" < "grid.bat" >"$TMP/${USER}_edited.tmp"
    mv "$TMP/${USER}_edited.tmp" "grid.bat" 
    chmod 755 "grid.bat"
    popd
  fi
}

function rebuild_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  popd
  build_cak0l
}

