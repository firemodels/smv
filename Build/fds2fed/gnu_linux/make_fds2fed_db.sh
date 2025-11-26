#!/bin/bash
rm -f *.o *.mod
make -f ../Makefile gnu_linux_db -j 4
