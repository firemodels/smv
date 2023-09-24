#!/bin/bash
file=$1
outfile=$file.out
# The absolute path to the directoy of this script
script_dir="$(dirname "$(realpath $0)")"
# The absolute path to base of the repo
base_dir="$(realpath "$script_dir/../..")"
# The absolute path to the Source directory. Include paths are made relative to
# this
source_dir="$(realpath "$base_dir/Source")"
# The absoulte path to the directory of the file we're analysing
file_dir="$(dirname "$file")"
clang-tidy "$file" --config-file="$base_dir"/.clang-tidy -- -I"$source_dir"/shared -I"$file_dir" -I"$source_dir"/glew -I"$source_dir"/glut_gl -I"$source_dir"/pthreads -I"$source_dir"/zlib128 -I"$source_dir"/gd-2.0.15 -D_CRT_SECURE_NO_WARNINGS >& "$outfile"
