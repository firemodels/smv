#!/bin/bash

CURDIR=`pwd`
cd  ../../../Source
SOURCE_DIR=`pwd`
cd $CURDIR

cd ../../../Build/LIBS
LIBSDIR=`pwd`
cd $CURDIR

SMV_MAKE_OPTS=
TEST=
SANITIZE=
if [ "`uname`" == "Darwin" ]; then
  GLUT=
  GLIBDIROPT=
  if [ "$GLIBDIR" != "" ]; then
    GLIBDIROPT=-L$GLIBDIR
  fi
else
  GLUT=glut
fi

# define INTEL_ICC, INTEL_ICPP, GCC and GXX variables
source $SOURCE_DIR/scripts/set_compilers.sh

export COMPILER=$INTEL_ICC
export COMPILER2=$INTEL_ICPP

BUILD_LIBS=
BUILD_ALL=1
GLTYPE=COCOA
TESTOPT=
while getopts 'CfGhil:LprSX' OPTION
do
case $OPTION in
  C)
   COMPILER=clang
   COMPILER2=clang++
  ;;
  f)
   GLUT=freeglut
  ;;
  G)
   COMPILER=$GCC
   COMPILER2=$GXX
  ;;
  h)
  echo ""
  echo "options:"
  echo "-h - show this help info"
  echo "-i - incremental build"
  echo "-L - rebuild all libraries"
  echo "-p - build a profiling version of smokeview"
  echo "-S - build smokeview with sanitize debug options"
  exit
  ;;
  i)
   BUILD_ALL=
  ;;
  l)
   export SMV_LIBDIR=$LIBSDIR/"$OPTARG";
  ;;
  L)
   BUILD_LIBS=1
  ;;
  p)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILEFLAG=\"-pg\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILESTRING=\"p\" "
  ;;
  r)
  ;;
  S)
   SANITIZE=1
  ;;
  X)
   GLTYPE=XQUARTZ
  ;;
esac
done

export SMV_MAKE_OPTS
export GLUT
export TEST
export SANITIZE
export GLTYPE

# this parameter is only for the mac
if [ "`uname`" == "Darwin" ]; then
  export GLIBDIROPT
fi
