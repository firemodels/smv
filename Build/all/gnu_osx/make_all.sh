#!/bin/bash
set -euxo pipefail

BUILDDIR=$(pwd)/../..
BUILD=gnu_osx

function build_target() {
    TARGET=$1
    pushd "$BUILDDIR"/"$TARGET"/$BUILD || exit 1
    ./make_"${TARGET}".sh
    popd
}

build_target background
build_target convert
build_target env2mod
build_target fds2fed
build_target flush
# build_target get_time - only runs on windows
build_target pnginfo
# build_target set_path - only runs on windows
build_target smokediff
build_target smokeview
build_target smokezip
build_target timep
build_target wind2fds
