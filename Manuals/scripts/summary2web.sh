#!/bin/bash
URLBASE=/opt/www/html
CURDIR=`pwd`
cd ../../../
ROOTDIR=`pwd`
BASE=`basename $ROOTDIR`
EXT="${BASE##*_}"
if [ "$EXT" != "" ]; then
  BASE=$EXT
fi
BASE=$URLBASE/`whoami`/$BASE
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
