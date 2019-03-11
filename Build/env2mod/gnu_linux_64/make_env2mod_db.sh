#!/bin/bash

make -f ../Makefile clean
eval make -j 4 -f ../Makefile gnu_linux_64_db

