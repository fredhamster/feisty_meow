#!/usr/bin/env bash

# updates a vcxproj that had been converted from prior visual studio 2005
# to the newer 2010.

parms=($*)

outdir="$HOME/fixed_proj"

if [ ! -d "$outdir" ]; then mkdir -p $outdir; fi

for i in ${parms[*]}; do
  curr_parm="$i"
  base=$(basename "$curr_parm")
  echo fixing $base

  cat "$curr_parm" |
    sed -e 's/<PlatformToolset>v80<\/PlatformToolset>/<PlatformToolset>v100<\/PlatformToolset>/g' |
    sed -e 's/ide_files/build/g' |
    sed -e 's/release_[de][lx][el]/release/g' |
    sed -e 's/debug_[de][lx][el]/debug/g' |
    sed -e 's/<IntDir .*<\/IntDir>//g' |
    sed -e 's/<OutDir .*<\/OutDir>//g' |
    sed -e 's/<OutputPath.*<\/OutputPath>//g' |
    sed -e 's/<TargetFrameworkVersion>v2.0<\/TargetFrameworkVersion>/<TargetFrameworkVersion>v4.0<\/TargetFrameworkVersion>/g' |
    sed -e 's/\.\.\\\.\.\\\.\.\\build/\.\.\\\.\.\\\.\.\\\.\.\\\.\.\\build/g' |
    sed -e 's/\.\.\\\.\.\\lib_src/\.\.\\\.\.\\\.\.\\\.\.\\\.\.\\libraries/g' |
    cat >"$outdir/$base"

done

