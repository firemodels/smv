#!/bin/bash

directory=$1
tag=$2

cd ~/$directory

git checkout master
if [ "$tag" != "latest" ]; then
  if [ "$tag" != "" ]; then
    git branch -d release >& /dev/null
    git checkout $tag -b release
  fi
fi
