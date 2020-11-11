#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

# use -I to force use of the gnu compiler
OPTS="-I $*"
source ../../../Source/scripts/setopts.sh $OPTS

if [[ "$target" == "all" ]] || [[ "$target" == "clean" ]]; then
rm *.a
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
  echo
  echo "********** building freeglut"
  echo
  cd $BUILDDIR/freeglut3.0.0/gnu_linux_64
  ./make_freeglut.sh $OPTS
else
  echo
  echo "********** building glut"
  echo
  cd $SRCDIR/glut-3.7.6
  ./makelib.sh $OPTS
fi
cp libglut.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "glui" ]]; then
# GLUI
echo
echo "********** building glui"
echo
cd $SRCDIR/glui_v2_1_beta
./makelib.sh $OPTS
cp libglui.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "zlib" ]]; then
# ZLIB
echo
echo "********** building zlib"
echo
cd $SRCDIR/zlib128
./makelib.sh $OPTS
cp libz.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "jpeg" ]]; then
# JPEG
echo
echo "********** building jpeg"
echo
cd $SRCDIR/jpeg-9b
./makelib.sh $OPTS
cp libjpeg.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "png" ]]; then
# PNG
echo
echo "********** building png"
echo
cd $SRCDIR/png-1.6.21
./makelib.sh $OPTS
cp libpng.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$target" == "gd" ]]; then
# GD
echo
echo "********** building gd"
echo
cd $SRCDIR/gd-2.0.15
./makelib.sh $OPTS
cp libgd.a $LIBDIR/.
fi

if [[ "$target" == "all" ]] || [[ "$arg" == "lua" ]]; then
# LUA variable is set by passing -l to this script (lower case L)
if [ "$LUA" == "lua" ]; then

# Lua # Lua interpreter
cd $SRCDIR/lua-5.3.1/src
export TARGET=liblua.a
./makelib.sh $OPTS
cp liblua.a $LIBDIR/.

# LPEG # Lua parsing libarary to parse SSF files
cd $SRCDIR/lpeg-1.0.0
export TARGET=linux
./makelib.sh $OPTS
cp lpeg.so $LIBDIR/.
fi
fi
