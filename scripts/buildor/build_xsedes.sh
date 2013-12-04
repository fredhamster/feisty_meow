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

# clean up some things.
#maybe not needed.
if [ ! -d unit-test-reports ]; then
echo this chunk in build_xsedes could be removed to clean up unit tests
else
\rm -rf unit-test-reports
fi

  # build the trunk.
  ant -Dbuild.targetArch=64 build
  if [ $? -ne 0 ]; then return 1; fi
  echo "Build done at: $(date)"
  popd

  success_sound  
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

# a shortcut for doing a new build and creating a bootstrap container with it.
function rebu_bootstrap()
{
  rebuild_xsede 
  check_result "failed to rebuild xsede code"

  bash $GENII_INSTALL_DIR/xsede_tools/library/bootstrap_quick_start.sh
  check_result "failed to bootstrap a container"

  success_sound  
}


