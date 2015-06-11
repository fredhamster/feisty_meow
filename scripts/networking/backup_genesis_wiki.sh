#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

zip -r "/z/stuffing/archives/genesis2_wiki_as_of_$(date_stringer).zip" /z/uvaweb/doc/wiki -x "*/xcg_releases/*" -x "*/xsede_beta/*" -x "*/GenesisII*gz" -x "*/act126_installers/*" -x "*/genesis2-*" -x "*/udc3_releases/*" -x "*/xsede_releases/*" -x "*/gffseu_releases/*"


