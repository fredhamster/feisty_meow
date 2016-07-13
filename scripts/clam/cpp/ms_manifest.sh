#!/bin/bash
# the basename is the file that needs its manifest stuffed into the file
# itself.  the where parameter tells us what index to use when stuffing it.
basename=$1
where=$2
if [ -z "$WIN32_MANIFEST_FILE" ]; then
  WIN32_MANIFEST_FILE=$CLAM_DIR/cpp/ms_manifests/security_manifest.txt 
fi
error_val=0
if [ -f "$basename.manifest" -a -f "$basename" ]; then 
  bash $BUILD_SCRIPTS_DIR/wrapdoze.sh mt -manifest $basename.manifest $WIN32_MANIFEST_FILE -outputresource:$basename\;$where >/dev/null 
  error_val=$?
elif [ -f "$basename" ]; then 
  bash $BUILD_SCRIPTS_DIR/wrapdoze.sh mt -manifest $WIN32_MANIFEST_FILE -outputresource:$basename\;$where >/dev/null
  error_val=$?
else
  echo skipping manifest generation for $basename.
  if [ ! -f "$basename.manifest" ]; then echo manifest file was missing.; fi
  if [ ! -f "$basename" ]; then echo main file was missing.; fi
fi
if [ $error_val -ne 0 ]; then
  echo There was an error attaching manifest to $1.
  exit 12
fi

