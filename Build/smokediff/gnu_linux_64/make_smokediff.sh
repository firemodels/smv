#!/bin/bash
rm -f *.o *.mod
make -f -j 4 ../Makefile gnu_linux_64
