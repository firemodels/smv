#!/bin/bash
rm -f *.o *.mod
make -f ../Makefile gnu_linux -j 4
