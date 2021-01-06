#!/bin/bash
rm -f *.o
make -j 4 -f ../Makefile intel_osx_64_db
