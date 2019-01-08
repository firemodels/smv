# !/bin/bash
# Exit immediately if any of the build steps fail
set -e
source ../../scripts/setopts.sh $*
LIBDIR=../../LIBS/gnu_linux_64/
LUA_SCRIPTING="LUA_SCRIPTING=true"
eval make -C ${LIBDIR} -j 4 ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f ../Makefile gnu_linux_64

