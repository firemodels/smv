#!/bin/bash
source ../../scripts/setopts.sh $*

OPEN_MPI=
if [ "$SMV_MPI" == "1" ]; then
  OPEN_MPI='OPEN_MPI="true"'
fi


LIBDIR=../../LIBS/intel_osx_64/
eval make -C ${LIBDIR} ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all_nolua_noglut

make -f ../Makefile clean
eval make -j 4 QUARTZ="$QUARTZ" $OPEN_MPI OPEN_MPI_ROOT="$MPIHOME" GLUT="$GLUT" ${SMV_MAKE_OPTS} -f ../Makefile intel_osx_64
