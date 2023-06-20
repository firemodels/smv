#!/bin/bash
# Exit immediately if any of the build steps fail
set -e

cd ../../../
cmake -B cbuild -DLUA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build cbuild --config Release
cmake --install cbuild --config Release --prefix dist
ctest --test-dir cbuild -C Release
