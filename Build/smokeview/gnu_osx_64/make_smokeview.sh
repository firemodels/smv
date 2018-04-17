#!/bin/bash
LIBDIR=../../LIBS/gnu_osx_64/
source ../../scripts/test_libs.sh

make -f ../Makefile clean
make -f ../Makefile gnu_osx_64
