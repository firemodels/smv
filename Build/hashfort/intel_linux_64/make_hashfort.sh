#!/bin/bash
rm -f *.o *.mod
source $IFORT_COMPILER/bin/compilervars.sh intel64
make -f ../Makefile intel_linux_64
