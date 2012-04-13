#!/bin/bash

function build_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant -Dbuild.targetArch=64 build
  # fix memory limits.
  sed -e "s/-Xmx512M/-Xmx2G/" < "runContainer.sh" >"$TMP/${USER}_edited.tmp"
  mv "$TMP/${USER}_edited.tmp" "runContainer.sh" 
  sed -e "s/-Xmx512M/-Xmx1G/" < "grid" >"$TMP/${USER}_edited.tmp"
  mv "$TMP/${USER}_edited.tmp" "grid" 
  chmod 755 "grid" "runContainer.sh" 
  popd
}

function rebuild_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  popd
  build_cak0l
}

