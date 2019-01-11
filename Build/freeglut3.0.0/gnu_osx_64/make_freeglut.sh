#!/bin/bash

rm *.o *.a
eval make -j 4 PLATFORM="x11" -f ../Makefile gnu_osx_64
