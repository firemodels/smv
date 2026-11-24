#!/bin/bash
# Pin the Apple Silicon build to arm64, including from a Rosetta shell.
if [ "$SMV_OSX_ARCH" = "arm64" ] || /usr/bin/arch -arm64 /usr/bin/true 2>/dev/null; then
  export SMV_LIB_ARCH=arm64
  export CMAKE_OSX_ARCHITECTURES=arm64
fi
exec ../make_LIBS_common.sh -C "$@"
