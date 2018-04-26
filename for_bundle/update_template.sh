#!/bin/bash

CURDIR=`pwd`
cd ..
ROOT=`pwd`
cd $CURDIR
MAKEPODIR=$ROOT/Build/makepo/intel_linux_64
MAKEPO=$MAKEPODIR/makepo_linux_64
SMVDIR=$ROOT/Source/smokeview
SHAREDDIR=$ROOT/Source/shared
OUT=smokeview_template.po

if [ ! -e $MAKEPO ]; then
  echo "***warning: The application $MAKEPO does not exist."
  echo "Building makepo"
  cd $MAKEPODIR
  ./make_makepo.sh
  if [ ! -e $MAKEPO ]; then
    echo "***error: The application $MAKEPO failed to build. Script aborted."
    cd $CURDIR
    exit
  fi
  cd $CURDIR
fi

echo updating smokeview_template.po
cat $SMVDIR/*.h $SHAREDDIR/*.h $SMVDIR/*.c $SMVDIR/*.cpp $SHAREDDIR/*.c | $MAKEPO | sort -f -u | $MAKEPO -a > $OUT
