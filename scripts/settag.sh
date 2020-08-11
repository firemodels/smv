#!/bin/bash

directory=$1
tag=$2
name=$3

cd ~/$directory

git checkout master
if [ "$tag" != "latest" ]; then
  if [ "$tag" != "" ]; then
    if [ "$name" != "" ]; then
      name="-b $name"
    fi
    echo git checkout $tag $name
    git checkout $tag $name
  fi
fi
