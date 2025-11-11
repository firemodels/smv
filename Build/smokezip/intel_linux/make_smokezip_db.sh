#!/bin/bash
rm -f *.o *.mod  smokezip_linux
make -j 4 -f ../Makefile intel_linux_db
