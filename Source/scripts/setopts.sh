#!/bin/bash
COMPILER="icc"
COMPILER2="icc"
QUARTZ=
PLATFORM=""
while getopts 'ghiq' OPTION
do
case $OPTION in
  g)
   COMPILER="gcc"
   COMPILER2="g++"
  ;;
  h)
  echo "options:"
  echo "-g - use the gcc compiler"
  echo "-i - use the Intel icc compiler"
  echo "-q - using Quartz X11"
  exit
  ;;
  i)
   COMPILER="icc"
   COMPILER2="icc"
  ;;
  q)
   QUARTZ="-I /opt/X11/include -Wno-unknown-pragmas"
  ;;
esac
done
shift $(($OPTIND-1))
if [ "`uname`" == "Darwin" ]; then
  PLATFORM="-D pp_OSX"
fi
export COMPILER
export PLATFORM
export QUARTZ
