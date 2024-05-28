#!/bin/bash
rm -f *.o *.mod fds2fed_osx_64
make -j 4 -f ../Makefile intel_osx_64
