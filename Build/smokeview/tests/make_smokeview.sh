#!/bin/bash
COMPILE=$1
GLUT=$2
PLATFORM="linux"
if [ `uname` == "Darwin" ]; then
  PLATFORM="osx"
fi

CURDIR=`pwd`
if [ "$COMPILE" != "clang" ]; then
  COMPILE=intel
fi
GLUTTYPE=
if [ "$GLUT" != "glut" ]; then
  GLUT=freeglut
  GLUTTYPE=-f
fi
echo "*** build smokeview using $COMPILE and $GLUT"
echo "*** cleaning repo"
cd ../../..
git clean -dxf > Nul
cd Build/smokeview/${COMPILE}_${PLATFORM}
./make_smokeview.sh $GLUTTYPE
./smokeview_$PLATFORM -v
echo "*** build complete"
