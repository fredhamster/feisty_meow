#!/bin/bash

#hmmm: NOT ported to testkit yet.

# creates a new certificate based on the grid's signing cert.
#
# Author: Chris Koeritz

##############

####

export WORKDIR="$( \cd "$(\dirname "$0")" && \pwd )"  # obtain the script's working directory.

# pull in the xsede test base support.
source "$WORKDIR/../prepare_tools.sh" "$WORKDIR/../prepare_tools.sh"

# if that didn't work, complain.
if [ -z "$TESTKIT_SENTINEL" ]; then echo Please run prepare_tools.sh before testing.; exit 3; fi

# load the bash libraries we need.
source "$TESTKIT_ROOT/library/helper_methods.sh"
#source "$TESTKIT_ROOT/library/runner_functions.sh"
source "$TESTKIT_ROOT/library/security_management.sh"

####

if [ $# -lt 7 ]; then
  echo "this script needs 7 parameters:"
  echo "1: signing cert PFX file to base new cert on."
  echo "2: the password for the signing cert PFX."
  echo "3: the alias of the key to use within the signing PFX."
  echo "4: output file to create with new certificate in PFX format."
  echo "5: password for new certificate PFX file."
  echo "6: alias for the certificate within the PFX file."
  echo "7: Common Name (CN) of the identity signified by the new certificate."
  echo
  echo "This is an example run using a signing cert for a container:"
  echo
  echo 'bash $TESTKIT_ROOT/library/gen_cert_from_ca.sh signing-cert.pfx signer signing-cert $HOME/new_cert.pfx myPassword certalias "Fred Powers"'
  exit 1
fi

signing_cert="$1"; shift
signing_passwd="$1"; shift
signing_alias="$1"; shift
output_file="$1"; shift
output_passwd="$1"; shift
output_alias="$1"; shift
output_cn="$1"; shift

create_pfx_using_CA "$signing_cert" "$signing_passwd" "$signing_alias" "$output_file" "$output_passwd" "$output_alias" "$output_cn"
check_if_failed "generating '$output_file' from '$signing_cert'"

echo "New certificate was generated into: $output_file"

exit 0

