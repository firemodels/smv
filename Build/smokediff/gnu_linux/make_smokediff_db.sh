#!/bin/bash
rm -f *.o *.mod
make -f ../Makefile -j 4 gnu_linux_db
