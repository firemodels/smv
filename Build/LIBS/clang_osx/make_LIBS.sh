#!/bin/bash
# Pin the Apple Silicon build to arm64, including from a Rosetta shell.
if [ "$(sysctl -n hw.optional.arm64 2>/dev/null)" = "1" ]; then
  export SMV_LIB_ARCH=arm64
  export CMAKE_OSX_ARCHITECTURES=arm64
fi
exec ../make_LIBS_common.sh -C "$@"
