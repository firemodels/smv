#!/bin/bash
COMPILER="icc"
COMPILER2="icc"
PLATFORM=""
GLUT=glut
while getopts 'fghi' OPTION
do
case $OPTION in
  f)
   GLUT="freeglut"
  ;;
  g)
   COMPILER="gcc"
   COMPILER2="g++"
  ;;
  h)
  echo "options:"
  echo "-g - use the gcc compiler"
  echo "-i - use the Intel icc compiler"
  exit
  ;;
  i)
   COMPILER="icc"
   COMPILER2="icc"
  ;;
esac
done
shift $(($OPTIND-1))
if [ "`uname`" == "Darwin" ]; then
  PLATFORM="-D pp_OSX"
fi
export COMPILER
export PLATFORM
export GLUT
