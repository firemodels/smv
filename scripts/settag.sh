#!/bin/bash

directory=$1
tag=$2

cd ~/$directory

git checkout master
if [ "$tag" != "latest" ]; then
if [ "$tag" != "" ]; then
  git checkout $tag
fi
fi
