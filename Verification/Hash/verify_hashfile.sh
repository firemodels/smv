#!/bin/bash

platform="linux"
if [ "`uname`" == "Darwin" ]; then
  platform="osx"
fi

HASHFILE=../../Build/hashfile/intel_${platform}_64/hashfile_${platform}_64
if [ ! -e $HASHFILE ]; then
  echo "***error: the program $HASHFILE does not exist"
  echo "verification aborted"
  exit
fi

$HASHFILE -hash_all base > result
ndiff=`diff result reference_$platform | wc -l`
if [ $ndiff -eq 0 ]; then
  echo PASS! The hashes computed by hashfile match the reference file.
else
  echo FAIL! The hashes computed by hashfile do not match the reference file.
fi
