#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

#hmmm: if this works well, we can use it in lots of places.
define_yeti_alias BAIL_ON_FAIL='if [ $? -ne 0 ]; then echo "A problem occurred.  $msg"; return 1; fi'

function zapem()
{
  bash $XSEDE_TEST_ROOT/library/zap_genesis_javas.sh 
}

# a macro for testing the configuration.
export GFFS_CHECK_VARS='
  if [ -z "$GENII_INSTALL_DIR" -o -z "$XSEDE_TEST_ROOT" ]; then
    echo "GENII_INSTALL_DIR is not set.";
    return 1;
  fi '

function build_gffs()
{
  eval $GFFS_CHECK_VARS
#  if [ -z "$GENII_INSTALL_DIR" -o -z "$XSEDE_TEST_ROOT" ]; then
#    echo GENII_INSTALL_DIR is not set.
#    return 1
#  fi
  zapem
  pushd "$GENII_INSTALL_DIR"
  if [ $? -ne 0 ]; then
    error_sound
    return 1
  fi
  echo "Build starting at: $(date)"

  # clean up some things.
  \rm -rf unit-test-reports

  # build the trunk.
  ant -Dbuild.targetArch=64 build
  if [ $? -ne 0 ]; then
    error_sound
    return 1
  fi
  echo "Build done at: $(date)"
  popd

  success_sound  
}

function rebuild_gffs()
{
  eval $GFFS_CHECK_VARS
#  if [ -z "$GENII_INSTALL_DIR" -o -z "$XSEDE_TEST_ROOT" ]; then
#    echo GENII_INSTALL_DIR is not set.
#    return 1
#  fi
  zapem
  pushd "$GENII_INSTALL_DIR"
  ant clean

  if [ $? -ne 0 ]; then
    error_sound
    return 1
  fi
  popd
  build_gffs
}

# a shortcut for doing a new build and creating a bootstrap container with it.
function rebu_bootstrap()
{
  eval $GFFS_CHECK_VARS
  rebuild_gffs 
  if [ $? -ne 0 ]; then echo "failed to rebuild gffs code"; return 1; fi

  quickstarter="$GENII_INSTALL_DIR/xsede_tools/library/bootstrap_quick_start.sh"
  if [ ! -f "$quickstarter" ]; then
    quickstarter="$XSEDE_TEST_ROOT/library/bootstrap_quick_start.sh"
  fi

  bash "$quickstarter"
  if [ $? -ne 0 ]; then
    echo "failed to bootstrap a container."
    error_sound
    return 1
  fi

  success_sound  
}

# a shortcut for building without a clean, and creating a bootstrap container with the code.
function bu_bootstrap()
{
  eval $GFFS_CHECK_VARS
  build_gffs 
  if [ $? -ne 0 ]; then echo "failed to rebuild gffs code"; return 1; fi

  quickstarter="$GENII_INSTALL_DIR/xsede_tools/library/bootstrap_quick_start.sh"
  if [ ! -f "$quickstarter" ]; then
    quickstarter="$XSEDE_TEST_ROOT/library/bootstrap_quick_start.sh"
  fi

  bash "$quickstarter"
  if [ $? -ne 0 ]; then
    echo "failed to bootstrap a container."
    error_sound
    return 1
  fi

  success_sound  
}

# a shortcut for doing a quick build and then creating an installer.
function fast_install_build()
{
  eval $GFFS_CHECK_VARS
  bash "$XSEDE_TEST_ROOT/tools/installer/fast_installer_build.sh" $*
  if [ $? -ne 0 ]; then
    echo "failed to create the installer."
    error_sound
    return 1
  fi

  success_sound  
}


