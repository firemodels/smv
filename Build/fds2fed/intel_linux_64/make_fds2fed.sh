#!/bin/bash
rm -f *.o *.mod fds2fed_linux_64
make -j 4 -f ../Makefile intel_linux_64
