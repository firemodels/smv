#!/bin/bash
make -f ../Makefile clean
make -j 4 -f ../Makefile intel_osx_64_db
