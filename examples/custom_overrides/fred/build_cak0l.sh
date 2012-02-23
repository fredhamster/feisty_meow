#!/bin/bash

function rebuild_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  ant -Dbuild.targetArch=64 build
  popd
}

function build_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant -Dbuild.targetArch=64 build
  popd
}


