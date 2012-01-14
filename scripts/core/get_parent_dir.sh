#!/bin/bash

#not perfect--if the thing has not path in front at all, not even a '.',
#then the code below fails since it can't cd into that portion.

absolute_path=$(cd ${0%/*} && echo $PWD/${0##*/})

path_only=$(dirname "$absolute_path")

echo absolute path to item: $absolute_path
echo just the directory: $path_only
