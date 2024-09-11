#!/bin/bash
URLBASE=$1
if [ "$URLBASE" == "" ]; then
  URLBASE=/opt/www/html
fi
if [ ! -d $URLBASE ]; then
  echo "***error: $URLBASE does not exist"
  exit
fi
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
  echo "***error: failed to create directory $BASE"
  exit
fi
cd $CURDIR/../SMV_Summary
SUMDIR=`pwd`
cp -r * $BASE/.
echo $SUMDIR copied to $BASE
