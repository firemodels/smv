#!/bin/bash
# use -I to force use of the Intel compiler
OPTS="-I $*"
source ../../../Source/scripts/setopts.sh $OPTS

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

LIBDIR=`pwd`
rm *.a

SRCDIR=$LIBDIR/../../../Source
cd $SRCDIR
SRCDIR=`pwd`

cd ../Build
BUILDDIR=`pwd`

# GD
echo
echo "********** building GD" 
echo
cd $SRCDIR/gd-2.0.15
./makelib.sh $OPTS
cp libgd.a $LIBDIR/.

# GLUI
echo
echo "********** building glui"
echo
cd $SRCDIR/glui_v2_1_beta
./makelib.sh $OPTS
cp libglui.a $LIBDIR/.

# GLUT
if [ "$GLUT" == "freeglut" ]; then
  cd $BUILDDIR/freeglut3.0.0/intel_osx_64
  ./make_freeglut.sh $OPTS 
  cp libglut.a $LIBDIR/.
else
  if [ "$QUARTZ" != "framework" ]; then
    cd $SRCDIR/glut-3.7.6
    ./makelib.sh $OPTS
    cp libglut.a $LIBDIR/.
  fi
fi

# JPEG
echo
echo "********** building jpeg"
echo
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS
cp libjpeg.a $LIBDIR/.

# PNG
echo
echo "********** building png"
echo
cd $SRCDIR/png-1.6.21
./makelib.sh $OPTS
cp libpng.a $LIBDIR/.

# ZLIB
echo
echo "********** building zlib"
echo
cd $SRCDIR/zlib128
./makelib.sh $OPTS
cp libz.a $LIBDIR/.

if [ "$LUA" == "lua" ]; then

# Lua # Lua interpreter
cd $SRCDIR/lua-5.3.1/src
export TARGET=liblua.a
./makelib.sh $OPTS
cp liblua.a $LIBDIR/.

# LPEG # Lua parsing libarary to parse SSF files
cd $SRCDIR/lpeg-1.0.0
export TARGET=macosx
./makelib.sh $OPTS
cp lpeg.so $LIBDIR/.
fi

