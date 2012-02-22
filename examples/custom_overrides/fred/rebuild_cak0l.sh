#!/bin/bash
pushd ~/xsede/code/cak0l/trunk
ant clean
ant -Dbuild.targetArch=64 build
popd

