#!/bin/bash
rm -f *.o *.mod fds2fed_linux
make -j 4 -f ../Makefile intel_linux
