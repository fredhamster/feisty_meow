#!/usr/bin/env bash

# get the uva codes.
mkdir -p ~/data/code_depot/uva_codes/
rsync -avz --delete cak0l@clambook:apps/  ~/data/code_depot/uva_codes/

