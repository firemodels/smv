#!/bin/bash
rm -f *.o
make -j4 -f ../Makefile clang_linux_64
