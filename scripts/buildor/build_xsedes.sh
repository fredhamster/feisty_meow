#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

#hmmm: if this works well, we can use it in lots of places.
alias BAIL_ON_FAIL='if [ $? -ne 0 ]; then echo "A problem occurred.  $msg"; return 1; fi'

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
  if [ $? -ne 0 ]; then echo "failed to rebuild xsede code"; return 1; fi

  bash $GENII_INSTALL_DIR/xsede_tools/library/bootstrap_quick_start.sh
  if [ $? -ne 0 ]; then echo "failed to bootstrap a container"; return 1; fi

  success_sound  
}

# a shortcut for doing a quick build and then creating an installer.
function fast_install_build()
{
  build_xsede 
  if [ $? -ne 0 ]; then echo "failed to build xsede code"; return 1; fi

  bash $GENII_INSTALL_DIR/xsede_tools/tools/installer/fast_installer_build.sh $*
  if [ $? -ne 0 ]; then echo "failed to create the installer."; return 1; fi

  success_sound  
}


