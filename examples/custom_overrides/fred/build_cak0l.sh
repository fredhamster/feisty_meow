#!/bin/bash

function build_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant -Dbuild.targetArch=64 build
  popd
}

function rebuild_cak0l()
{
  pushd ~/xsede/code/cak0l/trunk
  ant clean
  popd
  build_cak0l
}

