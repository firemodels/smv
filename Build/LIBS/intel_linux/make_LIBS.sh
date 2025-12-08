#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

# use -I to force use of the gnu compiler
OPTS="-I $*"
source ../../../Source/scripts/setopts.sh $OPTS

if [[ "$target" == "all" ]] || [[ "$target" == "clean" ]]; then
  rm -f *.a
  if [ "$target" == "clean" ]; then
    exit
  fi
fi

LIBDIR=`pwd`

SRCDIR=$LIBDIR/../../../Source
cd $SRCDIR
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

# GLUT

if [[ "$target" == "all" ]] || [[ "$target" == "glut" ]]; then
  if [ "$GLUT" == "freeglut" ]; then
    echo "********** building freeglut"
    cd $BUILDDIR/freeglut3.0.0/gnu_linux >& freeglut.out
    ./make_freeglut.sh $OPTS >& $LIBDIR/freeglut.out &
    pid_glut=$!
  else
    echo "********** building glut"
    cd $SRCDIR/glut-3.7.6
    ./makelib.sh $OPTS >& $LIBDIR/glut.out &
    pid_glut=$!
  fi
fi

if [[ "$target" == "all" ]] || [[ "$target" == "glui" ]]; then
# GLUI
  echo "********** building glui"
  cd $SRCDIR/glui_v2_1_beta
  ./makelib.sh $OPTS >& $LIBDIR/glui.out &
  pid_glui=$!
fi

if [[ "$target" == "all" ]] || [[ "$target" == "zlib" ]]; then
# ZLIB
  echo "********** building zlib"
  cd $SRCDIR/zlib131
  ./makelib.sh $OPTS >& $LIBDIR/zlib.out &
  pid_zlib=$!
fi

if [[ "$target" == "all" ]] || [[ "$target" == "jpeg" ]]; then
# JPEG
  echo "********** building jpeg"
  cd $SRCDIR/jpeg-9b
  ./makelib.sh $OPTS >& $LIBDIR/jpeg.out &
  pid_jpeg=$!
fi

if [[ "$target" == "all" ]] || [[ "$target" == "png" ]]; then
# PNG
  echo "********** building png"
  cd $SRCDIR/png-1.6.48
  ./makelib.sh $OPTS >& $LIBDIR/png.out &
  pid_png=$!
fi

if [[ "$target" == "all" ]] || [[ "$target" == "gd" ]]; then
# GD
  echo "********** building gd"
  cd $SRCDIR/gd-2.3.3
  ./makelib.sh $OPTS >& $LIBDIR/gd.out &
  pid_gd=$!
fi

#-------copy files

if [[ "$target" == "all" ]] || [[ "$target" == "glut" ]]; then
  wait $pid_glut
  echo glut library built
  cp libglut.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "glui" ]]; then
  wait $pid_glui
  echo glui library built
  cp libglui.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "zlib" ]]; then
  wait $pid_zlib
  echo zlib library built
  cp libz.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "jpeg" ]]; then
  wait $pid_jpeg
  echo jpeg library built
  cp libjpeg.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "png" ]]; then
  wait $pid_png
  echo png library built
  cp libpng.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "gd" ]]; then
  wait $pid_gd
  echo gd library built
  cp libgd.a $LIBDIR/.
fi
