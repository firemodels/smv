#!/bin/bash
CURDIR=`pwd`
cd ../../../
ROOTDIR=`pwd`
BASE=`basename $ROOTDIR`
BASE=/opt/www/html/`whoami`/$BASE
if [ ! -d $BASE ]; then
  mkdir $BASE
fi
if [ ! -d $BASE ]; then
  echo failed to create directory $BASE
  exit
fi
cd $CURDIR/../SMV_Summary
SUMDIR=`pwd`
cp -r * $BASE/.
echo $SUMDIR copied to $BASE
