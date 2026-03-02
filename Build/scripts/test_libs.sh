#!/bin/bash

BUILDLIBS=
LIBS="libgd.a libglui.a libjpeg.a libpng.a libz.a"
if [ "$GLUT" == "freeglut" ]; then
  LIBS="$LIBS libfreeglut.a"
else
  if [ "`uname`" != "Darwin" ]; then
    LIBS="$LIBS libglut.a"
  fi
fi

for lib in $LIBS; do
  if [ ! -e $SMV_LIBDIR/$lib ]; then
    BUILDLIBS=1
    break
  fi
done

if [ "$BUILDLIBS" != "" ]; then
  if [ -d $SMV_LIBDIR ]; then
    cd $SMV_LIBDIR
    echo ./make_LIBS.sh $*
    ./make_LIBS.sh $*
  else
    echo "***error: directory $LIBDIR does not exist"
  fi
fi
