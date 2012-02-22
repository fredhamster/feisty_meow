#!/bin/bash
pushd ~/xsede/code/cak0l/trunk
ant -Dbuild.targetArch=64 build
popd

