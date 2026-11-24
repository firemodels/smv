#!/bin/bash
rm -f *.o *.mod  part2iso_linux
make -j 4 -f ../Makefile intel_linux_db
