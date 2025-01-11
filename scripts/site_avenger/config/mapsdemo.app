#!/usr/bin/env bash

# a special override for the mapsdemo site, which we want to put in as
# a subdomain of the cakelampvm domain.

export DOMAIN_NAME="${APPLICATION_NAME}.cakelampvm.com"

echo "$(date_stringer): *** overrode domain name as: $DOMAIN_NAME" >> "$SSM_LOG_FILE"

