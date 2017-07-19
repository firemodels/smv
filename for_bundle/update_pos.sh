#!/bin/bash
CURDIR=`pwd`
cd ..
ROOT=`pwd`
cd $CURDIR
MERGEPODIR=$ROOT/Build/mergepo/intel_linux_64_db
MERGEPO=$MERGEPODIR/mergepo_linux_64_db
if [ ! -e $MERGEPO ]; then
  echo "***warning: The application $MERGEPO does not exist."
  echo "Building mergepo"
  cd $MERGEPODIR
  ./make_mergepo.sh
  if [ ! -e $MERGEPO ]; then
    echo "***error: The application $MERGEPO failed to build. Script aborted."
    cd $CURDIR
    exit
  fi
  cd $CURDIR
fi
for pofile in smokeview_??.po; do
  echo updating $pofile
  $MERGEPO -c $pofile smokeview_template.po > $pofile.revised
done
