#!/bin/bash

casename=$1
scriptdir=`dirname "$0"`
CURDIR=`pwd`
cd $scriptdir
scriptdir=`pwd`
cd $CURDIR
echo $scriptdir
awkfile=$scriptdir/getminmax.awk

for file in $casenmame*bf*bnd
do
  echo $file
  minmax=`awk -f $awkfile $file`
  mv $file ${file}.bak
  echo $minmax > $file
done
