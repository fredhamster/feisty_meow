#!/usr/bin/env bash

# Creates an archive from the test scripts.
#
# Author: Chris Koeritz

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.
cd "$WORKDIR"
export SHOWED_SETTINGS_ALREADY=true
if [ -z "$TESTKIT_SENTINEL" ]; then
  source ../prepare_tools.sh ../prepare_tools.sh 
fi
source "$TESTKIT_ROOT/library/establish_environment.sh"

pushd "$TESTKIT_ROOT/.." &>/dev/null
justdir="$(basename "$TESTKIT_ROOT")"

date_string="$(date +"%Y_%b_%e_%H%M" | sed -e 's/ //g')"

EXCLUDES=(--exclude=".svn" --exclude="docs" --exclude="random*.dat" --exclude=gzip-1.2.4 --exclude=iozone3_397 --exclude="mount-*" --exclude="releases" --exclude="passwords.txt" --exclude="saved_deployment_info.txt" --exclude="generated_certs" --exclude="gridwide_certs" --exclude="testkit.config*" --exclude="inputfile.txt*")

tar -czf "$HOME/testkit_${date_string}.tar.gz" "$justdir" ${EXCLUDES[*]} 

popd &>/dev/null

