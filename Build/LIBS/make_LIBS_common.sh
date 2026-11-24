#!/bin/bash

BUILD_LIBRARY()
{
  local logfile=$1
  shift
  if [ "${SMV_LIVE_BUILD_OUTPUT:-}" == "1" ]; then
    set -o pipefail
    "$@" 2>&1 | tee "$logfile"
  else
    "$@" > "$logfile" 2>&1
  fi
}

GET_FREEGLUT_WARNINGS()
{
  if [ `uname` == "Darwin" ]; then
    FREEGLUTLIB=lib
  else
    FREEGLUTLIB=lib64
  fi
  FREEGLUTLIBDIR=$LIBDIR/../../../../libs/freeglut/$FREEGLUTLIB
  if [ -e $FREEGLUTLIBDIR/libglut.a ]; then
    echo "*** freeglut library built"
    cp $FREEGLUTLIBDIR/libglut.a $LIBDIR/libfreeglut.a
  else
    echo "*** warning: freeglut library not built"
  fi
}

GET_WARNINGS()
{
  file=$1
  warnfile=$LIBDIR/${file}.wrn
  outfile=$LIBDIR/${file}.out
  if [ -e $LIBDIR/${file}.out ]; then
    grep -i warning $outfile > $warnfile
  fi
  if [ -e $LIBDIR/lib${file}.a ]; then
    echo "*** ${file} library built"
  else
    echo "*** warning ${file} library not built"
  fi
  if [ ! -s $warnfile ]; then
    rm -f $warnfile
  fi
}

git clean -dxf
if [ "$target" == "clean" ]; then
  exit
fi

LIBDIR=`pwd`

SRCDIR=$LIBDIR/../../../Source/LIBS
cd $SRCDIR
SRCDIR=`pwd`

cd ../../Build
BUILDDIR=`pwd`

OPTS="$*"
source $BUILDDIR/scripts/setopts.sh $OPTS

# ZLIB
echo "*** building zlib"
cd $SRCDIR/zlib131
BUILD_LIBRARY "$LIBDIR/zlib.out" ./makelib.sh $OPTS &
pid_zlib=$!

# JPEG
echo "*** building jpeg"
cd $SRCDIR/jpeg-9b
BUILD_LIBRARY "$LIBDIR/jpeg.out" ./makelib.sh $OPTS &
pid_jpeg=$!

# PNG
echo "*** building png"
cd $SRCDIR/png-1.6.48
BUILD_LIBRARY "$LIBDIR/png.out" ./makelib.sh $OPTS &
pid_png=$!

# GD
echo "*** building gd"
cd $SRCDIR/gd-2.3.3
BUILD_LIBRARY "$LIBDIR/gd.out" ./makelib.sh $OPTS &
pid_gd=$!

# GLUT

if [ "$GLUT" == "freeglut" ]; then
  if [ -d $LIBDIR/../../../../freeglut ]; then
    PLATFORM=linux
    if [ "`uname`" == "Darwin" ]; then
      PLATFORM=osx
    fi
    if [ "`uname`" == "Darwin" ]; then
      echo "*** building freeglut(${GLTYPE})"
    else
      echo "*** building freeglut"
    fi
    if [ "$COMPILER" == "icx" ]; then
      cd $BUILDDIR/freeglut/intel_$PLATFORM
    elif [ "$COMPILER" == "gcc" ]; then
      cd $BUILDDIR/freeglut/gnu_$PLATFORM
    else
      cd $BUILDDIR/freeglut/clang_$PLATFORM
    fi
    BUILD_LIBRARY "$LIBDIR/freeglut.out" ./make_freeglut.sh $OPTS &
    pid_glut=$!
  else
    echo "*** error: the freeglut repo does not exist. Clone the "
    echo "           freeglut repo by using bot/Scripts/setup_repos.sh -3 "
  fi
else
  if [ "`uname`" == "Darwin" ]; then
    echo "*** using OSX provided glut"
  else
    echo "*** building glut"
    cd $SRCDIR/glut-3.7.6
    BUILD_LIBRARY "$LIBDIR/glut.out" ./makelib.sh $OPTS &
    pid_glut=$!
  fi
fi

# GLUI
if [ "$pid_glut" != "" ]; then
  wait $pid_glut
fi
echo "*** building glui"
cd $SRCDIR/glui_v2_1_beta
BUILD_LIBRARY "$LIBDIR/glui.out" ./makelib.sh $OPTS &
pid_glui=$!

#-------copy files

if [ "$pid_glut" != "" ]; then
  if [ "$GLUT" == "freeglut" ]; then
    GET_FREEGLUT_WARNINGS
  else
    cd $SRCDIR/glut-3.7.6
    cp libglut.a $LIBDIR/.
    GET_WARNINGS glut
  fi
fi

wait $pid_glui
cd $SRCDIR/glui_v2_1_beta
cp libglui.a $LIBDIR/.
GET_WARNINGS glui

wait $pid_zlib
cd $SRCDIR/zlib131
cp libz.a $LIBDIR/.
GET_WARNINGS z

wait $pid_jpeg
cd $SRCDIR/jpeg-9b
cp libjpeg.a $LIBDIR/.
GET_WARNINGS jpeg

wait $pid_png
cd $SRCDIR/png-1.6.48
cp libpng.a $LIBDIR/.
GET_WARNINGS png

wait $pid_gd
cd $SRCDIR/gd-2.3.3
cp libgd.a $LIBDIR/.
GET_WARNINGS gd
