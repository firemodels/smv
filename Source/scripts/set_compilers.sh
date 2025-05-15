#!/bin/bash
if [ "`uname`" == "Darwin" ]; then
# The Mac doesn't have new compilers
  if [ "$INTEL_ICC" == "" ]; then
    INTEL_ICC="icc"
  fi
  if [ "$INTEL_ICPP" == "" ]; then
    INTEL_ICPP="icpc"
  fi
else
  if [ "$INTEL_ICC" == "" ]; then
    INTEL_ICC="icx"
  fi
  if [ "$INTEL_ICPP" == "" ]; then
    INTEL_ICPP="icpx"
  fi
fi

# define gcc

if [ "$GCC" == "" ]; then
  GCC=gcc
  if [ -e /usr/local/bin/gcc-14 ]; then
    GCC=/usr/local/bin/gcc-14
  fi
  export GCC
fi

# define g++

if [ "$GXX" == "" ]; then
  GXX=g++
  if [ -e /usr/local/bin/g++-14 ]; then
    GXX=/usr/local/bin/g++-14
  fi
  export GXX
fi
export INTEL_ICC
export INTEL_ICPP
export GCC
export GXX
