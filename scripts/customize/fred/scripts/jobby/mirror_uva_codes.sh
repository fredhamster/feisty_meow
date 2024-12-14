#!/bin/bash

# get the uva codes.
mkdir -p ~/data/code_depot/uva_codes/
rsync -avz --del cak0l@clambook:apps/*  ~/data/code_depot/uva_codes/

