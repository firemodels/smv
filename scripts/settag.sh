#!/bin/bash

directory=$1
tag=$2

cd ~/$directory

git checkout master
git checkout $tag
