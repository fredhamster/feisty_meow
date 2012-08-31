#!/bin/bash

date_string="$(date +"%Y_%b_%e_%H%M" | sed -e 's/ //g')"

zip -r "$HOME/archives/genesis2_wiki_as_of_${date_string}" /web/genesis2.virginia.edu/doc/ -x "*/xcg_releases/*" -x "*/xsede_beta/*" -x "*/GenesisII*gz"


