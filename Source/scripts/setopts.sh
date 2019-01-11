#!/bin/bash
COMPILER="icc"
COMPILER2="icc"
PLATFORM=""
GLUT=glut
LUA=
FOREC_g=
FOREC_i=
QUARTZ=framework
while getopts 'fgGhiIlq' OPTION
do
case $OPTION in
  f)
   GLUT="freeglut"
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
  echo "-f - use freeglut (not glut)
  echo "-g - use the gnu gcc compiler"
  echo "-i - use the Intel icc compiler"
  echo "-l - use lua 
  echo "-q - on the Mac use the X11 include files and libraries supplied by Quartz"
  exit
  ;;
  i)
   if [ "$FORCE_g" == "" ]; then
     COMPILER="icc"
     COMPILER2="icc"
   fi
  ;;
  I)
   COMPILER="icc"
   COMPILER2="icc"
   FORCE_i=1
  ;;
  l)
   LUA=lua
  ;;
  q)
   QUARTZ="-I /opt/X11/include -Wno-unknown-pragmas"
  ;;
esac
done
shift $(($OPTIND-1))

# the parameter QUARTZ is only for the mac
if [ "`uname`" == "Darwin" ]; then
  PLATFORM="-D pp_OSX"
  export QUARTZ
else
  QUARTZ=
fi
export COMPILER
export PLATFORM
export GLUT
export LUA
