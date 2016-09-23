#!/bin/bash
# the target is the file that needs its manifest stuffed into the file
# itself.  the where parameter tells us what index to use when stuffing it.
target=$1; shift
where=$1; shift
if [ -z "$WIN32_MANIFEST_FILE" ]; then
  WIN32_MANIFEST_FILE=$CLAM_DIR/cpp/ms_manifests/security_manifest.txt 
fi
for ((count=1 ; count <= 10; count++)); do
  error_val=0
  if [ -f "$target.manifest" -a -f "$target" ]; then 
    bash $BUILD_SCRIPTS_DIR/wrapdoze.sh mt -manifest $target.manifest $WIN32_MANIFEST_FILE -outputresource:$target\;$where >/dev/null 
    error_val=$?
  elif [ -f "$target" ]; then 
    bash $BUILD_SCRIPTS_DIR/wrapdoze.sh mt -manifest $WIN32_MANIFEST_FILE -outputresource:$target\;$where >/dev/null
    error_val=$?
  else
    echo skipping manifest generation for $target.
    if [ ! -f "$target.manifest" ]; then echo manifest file was missing.; fi
    if [ ! -f "$target" ]; then echo main file was missing.; fi
    break
  fi
  if [ $error_val -ne 0 ]; then
    echo "Error attaching manifest to $target at try #$count."
  else
    break
  fi
done
if [ $error_val -ne 0 ]; then
  echo There was an error attaching manifest to $target.
  exit 12
fi

