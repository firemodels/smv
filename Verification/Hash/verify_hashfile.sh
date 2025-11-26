#!/bin/bash

CURDIR=`pwd`
platform="linux"
if [ "`uname`" == "Darwin" ]; then
  platform="osx"
fi

HASHFILEDIR=../../Build/hashfile/intel_${platform}
cd $HASHFILEDIR
HASHFILEDIR=`pwd`
HASHFILE=$HASHFILEDIR/hashfile_${platform}
cd $CURDIR

if [ ! -e $HASHFILE ]; then
  echo "***error: the program $HASHFILE does not exist"
  echo "verification aborted"
  exit
fi

$HASHFILE -hash_all base > result
ndiff=`diff result reference_$platform | wc -l`
if [ $ndiff -eq 0 ]; then
  echo PASS! The hashes computed by $HASHFILE match the reference file.
else
  echo FAIL! The hashes computed by $HASHFILE do not match the reference file.
fi
