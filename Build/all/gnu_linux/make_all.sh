#!/bin/bash
set -euxo pipefail

BUILDDIR=$(pwd)/../..
BUILD=gnu_linux

function build_target() {
    TARGET=$1
    pushd "$BUILDDIR"/"$TARGET"/$BUILD || exit 1
    ./make_"${TARGET}".sh
    popd
}

build_target fds2fed
build_target part2iso
build_target pnginfo
build_target smokeview
build_target wind2fds
