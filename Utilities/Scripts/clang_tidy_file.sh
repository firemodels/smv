#!/bin/bash
file=$1
outfile=$file.out

clang-tidy $file -- -I../shared -I. -I../glew >& $outfile
