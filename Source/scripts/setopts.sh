#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# define INTEL_ICC, INTEL_ICPP, GCC and GXX variables
source $SCRIPT_DIR/set_compilers.sh

export COMPILER=$INTEL_ICC
export COMPILER2=$INTEL_ICPP

PLATFORM=""
GLUT=glut
LUA=
FOREC_g=
FOREC_i=
target=all
while getopts 'fgGhiILlt:T' OPTION
do
case $OPTION in
  f)
   GLUT="freeglut"
  ;;
  g)
   if [ "$FORCE_i" == "" ]; then
     COMPILER=$GCC
     COMPILER2=$GXX
   fi
  ;;
  G)
   COMPILER=$GCC
   COMPILER2=$GXX
   FORCE_g=1
  ;;
  h)
  echo "options:"
  echo "-f - use freeglut (not glut)"
  echo "-g - use the gnu gcc compiler"
  echo "-i - use the Intel icc compiler"
  echo "-q - on the Mac use the X11 include files and libraries supplied by Quartz"
  echo "-t target - makefile target"
  exit
  ;;
  i)
   if [ "$FORCE_g" == "" ]; then
     if [ "`uname`" == "Darwin" ]; then
       COMPILER="icc"
       COMPILER2="icpc"
     else
       COMPILER=$INTEL_ICC
       COMPILER2=$INTEL_ICPP
     fi
   fi
  ;;
  I)
    if [ "`uname`" == "Darwin" ]; then
     COMPILER="icc"
     COMPILER2="icpc"
   else
     COMPILER=$INTEL_ICC
     COMPILER2=$INTEL_ICPP
   fi
   FORCE_i=1
  ;;
  l)
   LUA=lua
  ;;
  L)
   dummy=1
  ;;
  t)
   target="$OPTARG"
  ;;
  T)
   dummy=1
  ;;
esac
done
shift $(($OPTIND-1))

if [ "`uname`" == "Darwin" ]; then
  PLATFORM="-D pp_OSX"
  if [ "$LOWRES" != "" ]; then
    PLATFORM="$PLATFORM -D pp_OSX_LOWRES"
  fi
else
  PLATFORM="-D pp_LINUX"
fi
export COMPILER
export COMPILER2
export PLATFORM
export GLUT
export LUA
export target
