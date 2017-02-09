#!/bin/bash

echo "Updating walrus and musix from surya: raw mode without syncthing!"
rsync -avz surya:/z/walrus/* /z/walrus/
rsync -avz surya:/z/musix/* /z/musix/
rsync -avz surya:/z/imaginations/* /z/imaginations/

