#!/bin/bash

# a special override for the mapsdemo site, which we want to put in as
# a subdomain of the cakelampvm domain.

source "$WORKDIR/config/default.app"

export DOMAIN_NAME="${APPLICATION_NAME}.cakelampvm.com"


