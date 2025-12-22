#!/bin/bash

GET_WARNINGS()
{
  file=$1
  warnfile=$LIBDIR/${file}.wrn
  outfile=$LIBDIR/${file}.out
  if [ -e $LIBDIR/${file}.out ]; then
    grep -i warning $outfile > $warnfile
  else
    echo $outfile does not exist > $warnfile
  fi
  if [ ! -s $warnfile ]; then
    rm -f $warnfile
  fi
}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

# use -C to force use of the clang compiler
OPTS="-C $*"
source ../../../Source/scripts/setopts.sh $OPTS

if [[ "$target" == "all" ]] || [[ "$target" == "clean" ]]; then
  rm -f *.a *.out *.wrn
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
    cd $BUILDDIR/freeglut3.0.0/gnu_linux
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
  GET_WARNINGS glut
  echo glut library built
  if [ "$GLUT" == "freeglut" ]; then
    cd $BUILDDIR/freeglut3.0.0/gnu_linux
  else
    cd $SRCDIR/glut-3.7.6
  fi
  cp libglut.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "glui" ]]; then
  wait $pid_glui
  GET_WARNINGS glui
  echo glui library built
  cd $SRCDIR/glui_v2_1_beta
  cp libglui.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "zlib" ]]; then
  wait $pid_zlib
  GET_WARNINGS zlib
  echo zlib library built
  cd $SRCDIR/zlib131
  cp libz.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "jpeg" ]]; then
  wait $pid_jpeg
  GET_WARNINGS jpeg
  echo jpeg library built
  cd $SRCDIR/jpeg-9b
  cp libjpeg.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "png" ]]; then
  wait $pid_png
  GET_WARNINGS png
  echo png library built
  cd $SRCDIR/png-1.6.48
  cp libpng.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "gd" ]]; then
  wait $pid_gd
  GET_WARNINGS gd
  echo gd library built
  cd $SRCDIR/gd-2.3.3
  cp libgd.a $LIBDIR/.
fi
