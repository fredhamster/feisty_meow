#!/bin/bash

echo "Updating walrus and musix from surya"
rsync -avz surya:/z/walrus/* /z/walrus/
rsync -avz surya:/z/musix/* /z/musix/

