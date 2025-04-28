#!/bin/bash
makelibs()
{
  lib=$1
  if [ ! -e $LIBDIR/$lib ] ; then
    CURDIR=`pwd`
    cd $LIBDIR
    ./make_LIBS.sh
    cd $CURDIR
  fi
}

makelibs libgd.a
makelibs libglui.a
if [ "`uname`" == "Darwin" ]; then
  if [ "$GLUT" == "freeglut" ]; then
    makelibs libglut.a
  fi
else
  makelibs libglut.a
fi
makelibs libjpeg.a
makelibs libpng.a
makelibs libz.a
