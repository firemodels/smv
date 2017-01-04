#!/bin/bash
source $IFORT_COMPILER/bin/compilervars.sh intel64

rm -f *.o
make "FORTLIBDIR=$IFORT_COMPILER_LIB"  -f ../Makefile intel_osx_64
