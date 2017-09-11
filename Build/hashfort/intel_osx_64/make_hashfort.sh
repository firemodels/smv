#!/bin/bash

source $IFORT_COMPILER/bin/compilervars.sh intel64
rm -f *.o *.mod
make -f ../Makefile intel_osx_64
