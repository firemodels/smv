#!/bin/bash
rm -f *.o *.mod
make -f ../Makefile -j 4 clang_linux_64_db
