#!/bin/bash

# this file attempts to provide all of the variables needed for compiling
# with the microsoft visual studio compiler (dot net version).  it requires
# one environment variable (called VSxCOMNTOOLS) be set that points at the
# location of the common tools in the visual studio installation.
# The value of x can be 80, 90 or 100.

function print_usage {
  echo "The VS80COMNTOOLS, VS90COMNTOOLS and VS100COMNTOOLS variables are not set."
  echo "This usually means that the Visual Studio compiler is not installed."
  echo ""
}

if [ "$OPERATING_SYSTEM" == "WIN32" ]; then
  export PATH=$PRODUCTION_DIR/win32_help:$PATH
fi

# we try to use the most recent compiler location, and work backwards as
# needed for the supported range (10 = vs 2010, 9 = vs 2008, 8 = vs 2005).
export VSxTOOLS="$VS100COMNTOOLS"
if [ -z "$VSxTOOLS" ]; then
  export VSxTOOLS="$VS90COMNTOOLS"
  if [ -z "$VSxTOOLS" ]; then
    export VSxTOOLS="$VS80COMNTOOLS"
  fi
fi

if [ -z "$VSxTOOLS" ]; then
  print_usage
  return 33
fi
#echo "common tools dir is \"$VSxTOOLS\""
export VSxTOOLS="$(short_path "$VSxTOOLS" | tr "A-Z" "a-z" | sed -e 's/\\/\//g' | sed -e 's/^\(.\):/\/\1/' )"
#echo cleaned comn tools is $VSxTOOLS 

export VIS_STU_ROOT="$(echo $VSxTOOLS | sed -e 's/^\(.*\)\/[^\/]*\/[^\/]*[\/]$/\1/' | sed -e 's/^\(.\):/\/\1/' )"
export VSINSTALLDIR="$VIS_STU_ROOT"
#echo root of visual studio is $VSINSTALLDIR

export WINDIR="$(short_path "$WINDIR" | sed -e 's/\\/\//g' | sed -e 's/^\(.\):/\/\1/' )"
#echo cleaned windir is $WINDIR

#echo "prior path is $PATH"

export VCINSTALLDIR="$VSINSTALLDIR/VC"
export VSCOMMONROOT="$VSINSTALLDIR/Common7"
export VS_TOOLS_DIR="$VSCOMMONROOT/tools"
export DevEnvDir="$VSCOMMONROOT/IDE"
export MSVCDir="$VCINSTALLDIR"
export FrameworkDir="$WINDIR/Microsoft.NET/Framework"
export FrameworkVersion=v4.0.30319
#old export FrameworkSDKDir="$VSINSTALLDIR/SDK/v2.0"

export PLATFORM_DIR="$VCINSTALLDIR/PlatformSDK"
if [ ! -d "$PLATFORM_DIR" ]; then
  export PLATFORM_DIR="$(short_path "$PROGRAMFILES/Microsoft SDKs/Windows/v7.0A" | tr "A-Z" "a-z" | sed -e 's/^\(.*\)\/[^\/]*\/[^\/]*[\/]$/\1/' | sed -e 's/^\(.\):/\/\1/' )"
fi
export WindowsSdkDir="$PLATFORM_DIR"
#echo platform dir is $PLATFORM_DIR

#echo "path before is $PATH"
export PATH="$DevEnvDir:$VCINSTALLDIR/BIN:$VSxTOOLS:$VSxTOOLS/bin:$FrameworkDir/$FrameworkVersion:$FrameworkDir/v3.5:$VCINSTALLDIR/VCPackages:$VSINSTALLDIR/Common7/Tools:$PLATFORM_DIR/bin:$PATH"
#echo "path after is $PATH"

export INCLUDE="$VCINSTALLDIR/ATLMFC/INCLUDE:$VCINSTALLDIR/INCLUDE:$PLATFORM_DIR/include"
#:$FrameworkSDKDir/include"

export LIB="$VCINSTALLDIR/ATLMFC/LIB:$VCINSTALLDIR/LIB:$PLATFORM_DIR/lib"
#:$FrameworkSDKDir/lib"

# convert framework dir back or things yell like hell.
export FrameworkDir="$(echo $FrameworkDir | sed -e 's/^\/\(.\)[\\\/]\(.*\)$/\1:\\\2/' | tr "/" "\\" 2>/dev/null )"
  # the redirection of stderr to null is to get around an obnoxious cygwin
  # warning that seems to be erroneously bitching about backslashes.
#echo framedir now $FrameworkDir


