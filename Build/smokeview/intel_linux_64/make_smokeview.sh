#!/bin/bash
source ../../scripts/setopts.sh $*

# Exit immediately if any of the build steps fail
set -e

LIBDIR=../../LIBS/intel_linux_64/
eval make -C ${LIBDIR} ${SMV_MAKE_OPTS} ${LUA_SCRIPTING} -f make_LIBS.make all

INTEL_MPI=
if [ "$SMV_MPI" == "1" ]; then
  INTEL_MPI='INTEL_MPI="true"'
fi

make -f ../Makefile clean
eval make -j 4 ${SMV_MAKE_OPTS} ${INTEL_MPI} I_MPI_ROOT="$I_MPI_ROOT" -f ../Makefile intel_linux_64
