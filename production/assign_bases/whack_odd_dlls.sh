#!/usr/bin/env bash

DA_DIR="$RUNTIME_PATH/binaries"

rm -f $DA_DIR/AxInterop.*.dll \
  $DA_DIR/DevAge*dll \
  $DA_DIR/DevComponents*dll \
  $DA_DIR/devcomponents*dll \
  $DA_DIR/DirectShowLib* \
  $DA_DIR/Dundas*dll \
  $DA_DIR/ICreateDataGrid.dll \
  $DA_DIR/ICreateTypeProbe.dll \
  $DA_DIR/Interop.*.dll \
  $DA_DIR/libeay32.dll \
  $DA_DIR/MagicLibrary.dll \
  $DA_DIR/Microsoft.SqlServer.*.dll \
  $DA_DIR/NineRays.Win.Widgets.dll \
  $DA_DIR/SlimDX.dll \
  $DA_DIR/SourceGrid.Extensions.dll \
  $DA_DIR/SourceLibrary.dll \
  $DA_DIR/ssleay32.dll \
  $DA_DIR/ToolkitPro*.dll \
  $DA_DIR/xunit.dll \
  $DA_DIR/xunit.extensions.dll \
  $DA_DIR/*.vshost.exe

