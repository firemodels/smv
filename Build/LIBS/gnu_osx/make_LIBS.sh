#!/bin/bash
# use -G to force use of the gnu compiler
OPTS="-G $*"

source ../../../Source/scripts/setopts.sh $OPTS

LIBDIR=`pwd`
git clean -dxf

cd $LIBDIR/../../../Source
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

cd $SRCDIR
git clean -dxf

cd $BUILDDIR
git clean -dxf

echo

# GD
echo building GD library
cd $SRCDIR/gd-2.3.3
./makelib.sh $OPTS  >& $LIBDIR/gd.out &
pid_gd=$!


# GLUI
echo building glui library
cd $SRCDIR/glui_v2_1_beta
if [ "$GLUT" == "freeglut" ]; then
  ./makelib.sh $OPTS -f >& $LIBDIR/glui.out &
  pid_glui=$!
else
  ./makelib.sh $OPTS >& $LIBDIR/glui.out &
  pid_glui=$!
fi

# FREEGLUT
if [ "$GLUT" == "freeglut" ]; then
  echo building glut
  cd $BUILDDIR/freeglut3.0.0/gnu_osx
  ./make_freeglut.sh $OPTS >& $LIBDIR/glut.out &
  pid_glut=$!
else
  echo ***using OSX provided glut
fi

# JPEG
echo building jpeg library
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS >& $LIBDIR/jpeg.out &
pid_jpeg=$!

# PNG
echo building png library
cd $SRCDIR/png-1.6.48
./makelib.sh $OPTS >& $LIBDIR/png.out &
pid_png=$!

# ZLIB
echo building zlib library
cd $SRCDIR/zlib131
./makelib.sh $OPTS >& $LIBDIR/zlib.out &
png_zlib=$!

#-------------copying libraries

wait $pid_gd
cd $SRCDIR/gd-2.3.3
echo GD library built
cp libgd.a $LIBDIR/.

wait $pid_glui
cd $SRCDIR/glui_v2_1_beta
echo glui library built
cp libglui.a $LIBDIR/.

if [ "$GLUT" == "freeglut" ]; then
  wait $pid_glut
  cd $BUILDDIR/freeglut3.0.0/gnu_osx
  echo glut library built
  cp libglut.a $LIBDIR/.
fi

wait $pid_jpeg
cd $SRCDIR/jpeg-9b
echo jpeg library built
cp libjpeg.a $LIBDIR/.

wait $pid_png
cd $SRCDIR/png-1.6.48
echo png library built
cp libpng.a $LIBDIR/.

wait $pid_zlib
cd $SRCDIR/zlib131
echo zlib library built
cp libz.a $LIBDIR/.
