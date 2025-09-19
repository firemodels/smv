#!/bin/bash

make -f ../Makefile clean
eval make -j 4 -f ../Makefile clang_osx_64

