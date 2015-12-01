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

if [[ "$1" =~ .*help.* ]]; then
  print_usage
  exit 0
fi

function setup_visual_studio_variables()
{
  chmod 755 $PRODUCTION_DIR/win32_helper/*.exe
  export PATH="$(dos_to_unix_path $PRODUCTION_DIR)/win32_helper:$PATH"

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
    return 33
  fi
  export VSxTOOLS="$(short_path "$VSxTOOLS" | tr "A-Z" "a-z" | sed -e 's/\\/\//g' )"
#| sed -e 's/^\(.\):/\/\1/' )"
  
  export VIS_STU_ROOT="$(echo $VSxTOOLS | sed -e 's/^\(.*\)\/[^\/]*\/[^\/]*[\/]$/\1/' )"
#| sed -e 's/^\(.\):/\/\1/' )"
  export VSINSTALLDIR="$VIS_STU_ROOT"
  
  export WINDIR="$(short_path "$WINDIR" | tr A-Z a-z | sed -e 's/\\/\//g' )"
#| sed -e 's/^\(.\):/\/\1/' )"
  
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

#on hold:    export PLATFORM_DIR="$(short_path "$PROGRAMFILES/Microsoft SDKs/Windows/v7.0A" | tr "A-Z" "a-z" | sed -e 's/^\(.*\)\/[^\/]*\/[^\/]*[\/]$/\1/' )"
##| sed -e 's/^\(.\):/\/\1/' )"

    # guess at where we can find this damned directory in its short form.
#hmmm: this is needed until there's a replacement for short path, or we can build again.
    export PLATFORM_DIR="c:/progra~2/micros~1/windows/v7.0a"
    if [ ! -d "$PLATFORM_DIR" ]; then
      PLATFORM_DIR="c:/progra~1/micros~1/windows/v7.0a"
      if [ ! -d "$PLATFORM_DIR" ]; then
        PLATFORM_DIR="c:/progra~1/micros~2/windows/v7.0a"
        if [ ! -d "$PLATFORM_DIR" ]; then
          PLATFORM_DIR="c:/progra~2/micros~2/windows/v7.0a"
        fi
      fi
    fi

    if [ ! -d "$PLATFORM_DIR" ]; then
      echo "*** Failure to calculate the platform directory based on several attempts using c:\\program files\\microsoft sdks\\windows\\v7.0a as the basic pattern"
    fi
  

  fi
  export WindowsSdkDir="$PLATFORM_DIR"
  
  #echo "path before is $PATH"
  local filena
  for filena in "$DevEnvDir" "$VCINSTALLDIR/BIN" "$VSxTOOLS" "$VSxTOOLS/bin" "$FrameworkDir/$FrameworkVersion" "$FrameworkDir/v3.5" "$VCINSTALLDIR/VCPackages" "$VSINSTALLDIR/Common7/Tools" "$PLATFORM_DIR/bin"; do 
    export PATH="$PATH:$(dos_to_unix_path $filena)"
  done
  #echo "path after is $PATH"
  
  export INCLUDE="$VCINSTALLDIR/ATLMFC/INCLUDE:$VCINSTALLDIR/INCLUDE:$PLATFORM_DIR/include"
  #:$FrameworkSDKDir/include"
  
  export LIB="$VCINSTALLDIR/ATLMFC/LIB:$VCINSTALLDIR/LIB:$PLATFORM_DIR/lib"
  #:$FrameworkSDKDir/lib"
  
  # convert framework dir back or things yell like hell.
  export FrameworkDir=$(unix_to_dos_path $FrameworkDir)
    # the redirection of stderr to null is to get around an obnoxious cygwin
    # warning that seems to be erroneously complaining about backslashes.
  
  ##############
  
  echo "calculated variables for dos/windoze builds:"
  var VIS_STU_ROOT VSxTOOLS WINDIR VSxTOOLS VSINSTALLDIR PLATFORM_DIR FrameworkDir
  
  ##############
}

# run the above, very nasty, function.
setup_visual_studio_variables


