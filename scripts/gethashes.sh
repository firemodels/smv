#!/bin/bash

# get a sha1 hash for every executable file in a directory

argdir=$1

cd $argdir
for file in `find . -maxdepth 1 -type f ` ; do
if [ -x $file ]; then
  file=$(basename $file)
  hashfile $file
fi
done

cd $CURDIR
