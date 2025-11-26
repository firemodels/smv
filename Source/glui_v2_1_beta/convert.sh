#!/bin/bash
platform="linux"
if [ "`uname`" == "Darwin" ] ; then
  platform="osx"
fi

curdir=`pwd`
cd ../../Build

BUILDDIR=`pwd`
CONVERTDIR=$BUILDDIR/convert/intel_${platform}_
CONVERTPROG=convert_${platform}
CONVERT=$CONVERTDIR/$CONVERTPROG

if [ ! -e $CONVERT ]; then
  cd $CONVERTDIR
  echo "***warning: $CONVERTPROG does not exit"
  echo "            building $CONVERTPROG"
  ./make_convert.sh
  if [ ! -e $CONVERT ]; then
     cd $curdir
     echo "***error failed to build $CONVERTPROG"
     exit
  fi
fi

cd $curdir

$CONVERT < glut_hel20.c.base > glut_hel20.c
$CONVERT < glut_hel24.c.base > glut_hel24.c
$CONVERT < glut_hel36.c.base > glut_hel36.c
