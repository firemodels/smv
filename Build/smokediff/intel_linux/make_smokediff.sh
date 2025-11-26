#!/bin/bash
rm -f *.o *.mod smokediff_linux
make -j 4 -f ../Makefile intel_linux
