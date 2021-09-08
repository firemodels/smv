#!/bin/bash
rm -f *.o *.mod  smokezip_linux_64
make -j 4 -f ../Makefile intel_linux_64
