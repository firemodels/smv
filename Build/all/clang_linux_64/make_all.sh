#!/bin/bash
set -euxo pipefail

BUILDDIR=$(pwd)/../..
BUILD=clang_linux_64

function build_target() {
    TARGET=$1
    pushd "$BUILDDIR"/"$TARGET"/$BUILD || exit 1
    ./make_"${TARGET}".sh
    popd
}

# build_target background
# build_target convert
# build_target env2mod
# build_target fds2fed
# build_target flush
# build_target get_time
# build_target getdata
# build_target hashfile
# build_target hashfort
# build_target makepo
# build_target mergepo
build_target pnginfo
# build_target set_path
build_target smokediff
build_target smokeview
build_target smokezip
# build_target timep
# build_target wind2fds
