#!/bin/bash

CURDIR=`pwd`
cd ..
ROOT=`pwd`
cd $CURDIR
MAKEPODIR=$ROOT/Build/makepo/intel_linux_64
MAKEPO=$MAKEPODIR/makepo_linux_64
SMVDIR=$ROOT/Source/smokeview
SHAREDDIR=$ROOT/Source/shared
OUT=smokeview_template.pl

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
cat $SMVDIR/*.h $SHAREDDIR/*.h $SMVDIR/*.c $SMVDIR/*.cpp $SHAREDDIR/*.c | $MAKEPO | sort -u | awk '{if(NF==2){print}}' | $MAKEPO -a -c > $OUT
cat $SMVDIR/*.h $SHAREDDIR/*.h $SMVDIR/*.c $SMVDIR/*.cpp $SHAREDDIR/*.c | $MAKEPO | sort -u | awk '{if(NF==3){print}}'  | $MAKEPO -a >> $OUT
cat $SMVDIR/*.h $SHAREDDIR/*.h $SMVDIR/*.c $SMVDIR/*.cpp $SHAREDDIR/*.c | $MAKEPO | sort -u | awk '{if(NF==4){print}}'  | $MAKEPO -a >> $OUT
cat $SMVDIR/*.h $SHAREDDIR/*.h $SMVDIR/*.c $SMVDIR/*.cpp $SHAREDDIR/*.c | $MAKEPO | sort -u | awk '{if(NF>4){print}}'  | $MAKEPO -a >> $OUT
