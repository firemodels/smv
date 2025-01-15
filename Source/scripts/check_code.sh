#!/bin/bash
file=$1
clang --analyze -I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128 $file >& $file.out
