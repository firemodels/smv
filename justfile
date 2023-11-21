set windows-powershell := true
alias b := build
alias br := build-release

_default:
    @just --list

# Run the tests
test:
    ctest --test-dir cbuild -C Debug

# Build the debug binaries
build:
    cmake -B cbuild -DLUA=ON -DGLUI=ON -DLUA_BUILD_BINARY=ON -DLUA_UTIL_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_HOST_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
    cmake --build cbuild --config Debug -j6 -v
    cmake --install cbuild --config Debug --prefix dist-debug

# Build the release binaries
build-release:
    cmake -B cbuild -DLUA=ON -DGLUI=ON -DLUA_BUILD_BINARY=ON -DLUA_UTIL_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_HOST_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
    cmake --build cbuild --config Release -v -j6
    cmake --install cbuild --config Release --prefix dist

# Build release and create MSI package
package-windows: build-release
    candle "SMVLuaInstaller.wxs"
    light "SMVLuaInstaller.wixobj"

# Build release and create RPM package
package-rpm:
    ./buildrpm.sh

# Clean the ./dist folder
clean-dist:
    rm -r dist

# Clean everything
clean: clean-dist
    rm -r cbuild
