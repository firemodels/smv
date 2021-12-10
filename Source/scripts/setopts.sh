#!/bin/bash
COMPILER="icx"
COMPILER2="icpx"
PLATFORM=""
GLUT=glut
LUA=
FOREC_g=
FOREC_i=
target=all
QUARTZ="-I /opt/X11/include -Wno-unknown-pragmas"
while getopts 'CfgGhiILlqQt:T' OPTION
do
case $OPTION in
  f)
   GLUT="freeglut"
  ;;
  C)
   if [ "$FORCE_g" == "" ]; then
     COMPILER="icx"
     COMPILER2="icpx"
   fi
  ;;
  g)
   if [ "$FORCE_i" == "" ]; then
     COMPILER="gcc"
     COMPILER2="g++"
   fi
  ;;
  G)
   COMPILER="gcc"
   COMPILER2="g++"
   FORCE_g=1
  ;;
  h)
  echo "options:"
  echo "-C - use the Intel icx/C11 compiler"
  echo "-f - use freeglut (not glut)
  echo "-g - use the gnu gcc compiler"
  echo "-i - use the Intel icc compiler"
  echo "-l - use lua 
  echo "-q - on the Mac use the X11 include files and libraries supplied by Quartz"
  echo "-t target - makefile target"
  exit
  ;;
  i)
   if [ "$FORCE_g" == "" ]; then
     COMPILER="icx"
     COMPILER2="icpx"
   fi
  ;;
  I)
   COMPILER="icx"
   COMPILER2="icpx"
   FORCE_i=1
  ;;
  l)
   LUA=lua
  ;;
  L)
   dummy=1
  ;;
  q)
   QUARTZ="-I /opt/X11/include -Wno-unknown-pragmas"
  ;;
  Q)
   QUARTZ=framework
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

# the parameter QUARTZ is only for the mac
if [ "`uname`" == "Darwin" ]; then
  if [ "$QUARTZ" == "framework" ]; then
    PLATFORM="-D pp_OSX"
    if [ "$LOWRES" != "" ]; then
      PLATFORM="$PLATFORM -D pp_OSX_LOWRES"
    fi
  else
    PLATFORM="-D pp_OSX -D pp_QUARTZ $QUARTZ"
  fi
  export QUARTZ
else
  PLATFORM="-D pp_LINUX"
  QUARTZ=
fi
export COMPILER
export PLATFORM
export GLUT
export LUA
export target
