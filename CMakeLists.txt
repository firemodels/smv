# We select CMake version 3.25 as it allows us to use the LINUX variable.
cmake_minimum_required(VERSION 3.17)
# Set a policy to use the newer MSVC ABI selection
cmake_policy(SET CMP0091 NEW)
# Set a policy to use the newer approach to timestampes in FetchContent
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19")
    cmake_policy(SET CMP0110 NEW)
endif()
# Use FetchContent, this is used mostly to pull in Lua via git
include(FetchContent)
# Selected the multithreaded runtime when using MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(smv LANGUAGES C CXX)
# Use C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
# Requires adherence to this standard
set(CMAKE_C_STANDARD_REQUIRED TRUE)
# GNUInstallDirs is used to set the appropriate paths on linux (/usr/lib64 etc).
include(GNUInstallDirs)
# Pass a flag to to the source code. There's no great need for this but allows
# us to have conditional code based on the build method.
add_definitions(-Dpp_CMAKE)

# Options which can be configured (along with their defaults)
option(BETA "Include beta functionality" OFF)
option(LUA "Include lua scripting" OFF)
option(LUA_BUILD_BINARY "Build a lua intepreter" OFF)
option(LUA_UTIL_LIBS "Build LPEG and LFS, useful native libs for lua" OFF)
option(STRICT_CHECKS "Run additional strict checks" OFF)

# There are some C-based lua libraries that are generally distributed separately
# (e.g. via a linux distribution or luarocks). Sometimes (particularly on
# Windows) it makes sense to compile them here.
if (LUA_UTIL_LIBS)
    set(VENDOR_LFS TRUE)
    set(VENDOR_LPEG TRUE)
endif()

# Ensure all files are in the right place to run the tests
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Detect and set flags for Mac
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
endif()
if (UNIX AND NOT MACOSX)
    set(LINUX TRUE)
endif()
# Set flags for Windows
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32)
    add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB X64 _WIN32 WIN32 _CONSOLE)
endif()
# Set flags for Linux
if (LINUX)
    add_definitions(-Dpp_LINUX)
endif()


find_package(OpenGL REQUIRED)
if (NOT(WIN32))
    find_package(GLUT)
endif()
if (NOT GLUT_FOUND)
    add_subdirectory(Source/glut-3.7.6)
endif()
find_package(OpenGL REQUIRED)
find_package(GLEW)
# If GLEW is not found natively we compile it into smokeview directly later in
# this file rather than adding a subdirectory
find_package(JPEG)
if (NOT JPEG_FOUND)
    add_subdirectory(Source/jpeg-9b)
endif()
find_package(PNG)
if (NOT PNG_FOUND)
    add_subdirectory(Source/png-1.6.21)
endif()
find_package(ZLIB)
if (NOT ZLIB_FOUND)
    add_subdirectory(Source/zlib128)
endif()

# GLUI has been modified for smokeview, so we have to vendor it and can't use a
# native version.
add_subdirectory(Source/glui_v2_1_beta)

# This fails on Windows so we disable it for now
if (NOT(WIN32))
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(LIBGD IMPORTED_TARGET gdlib)
    endif()
endif()

if (NOT LIBGD_FOUND)
    add_subdirectory(Source/gd-2.0.15)
endif()

if (WIN32)
    find_package(PThreads4W)
    if (NOT PThreads4W_FOUND)
        add_subdirectory(Source/pthreads)
    endif()
endif()

if (LUA)
    if (VENDOR_LFS)
        add_subdirectory(Source/lfs)
    endif()
    if (VENDOR_LPEG)
        add_subdirectory(Source/lpeg)
    endif()
endif()


if (NOT MSVC)
    add_compile_options(-Wunused)
else()
    add_compile_options(/W3)
endif()
# This is a set of warnings that are not enabled by default but it would be good
# to satisfy these.
if(STRICT_CHECKS AND (NOT MSVC))
    # set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    # set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
    add_compile_options(-O2)
    add_compile_options(-fexceptions)
    add_compile_options(-grecord-gcc-switches)
    add_compile_options(-Wall)
    add_compile_options(-Werror=format-security)
    add_compile_options(-Wp,-D_FORTIFY_SOURCE=2)
    add_compile_options(-Wp,-D_GLIBCXX_ASSERTIONS )
    add_compile_options(-fstack-protector-strong)
    add_compile_options(-fasynchronous-unwind-tables)
    add_compile_options(-fstack-clash-protection)
    add_compile_options(-fcf-protection)

    add_compile_options(-Wno-unknown-pragmas)
    add_compile_options(-Wno-uninitialized)
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-overflow)
    add_compile_options(-Wno-format-truncation)
    add_compile_options(-Wno-nonnull)
    add_compile_options(-Wno-comment)
    if (NOT (CMAKE_C_COMPILER_ID STREQUAL IntelLLVM))
        add_compile_options(-Wno-stringop-truncation)
        add_compile_options(-ffat-lto-objects)
        add_compile_options(-flto=auto)
    else()
        # IntelLLVM-specific options
        add_compile_options(-Wno-tautological-constant-compare)
        add_compile_options(-Wno-for-loop-analysis)
    endif()

    add_compile_options(-Wshadow)
elseif(STRICT_CHECKS AND (NOT MSVC))
    add_compile_options(/Wall)
    add_compile_options(/Werror)
endif()


if (BETA)
    add_compile_definitions(pp_BETA)
endif ()


include(CTest)
enable_testing()
add_subdirectory(Tests)
option(VERIFICATION_TESTS "Run larger verification tests" OFF)
if (VERIFICATION_TESTS)
    add_subdirectory(Verification/Visualization)
endif()

# Retrieve configuration and data files from the bot repo
file(DOWNLOAD "https://raw.githubusercontent.com/firemodels/bot/a0a9d710f7a55b4e04c9ac4e32a04e2efe197c47/Bundlebot/smv/for_bundle/objects.svo" "${CMAKE_BINARY_DIR}/objects.svo" STATUS object_svo_status)
list(GET object_svo_status 0 object_svo_status_code)
if (object_svo_status_code)
    message(FATAL_ERROR "Could not download objects.svo ${smokeview_ini_status_code}")
endif()

file(DOWNLOAD "https://raw.githubusercontent.com/firemodels/bot/a0a9d710f7a55b4e04c9ac4e32a04e2efe197c47/Bundlebot/smv/for_bundle/smokeview.ini" "${CMAKE_BINARY_DIR}/smokeview.ini" STATUS smokeview_ini_status)
list(GET smokeview_ini_status 0 smokeview_ini_status_code)
if (smokeview_ini_status_code)
    message(FATAL_ERROR "Could not download smokeview.ini ${smokeview_ini_status_code}")
endif()

# Libraries to build (which may be dependencies of the programs below)
add_subdirectory(Source/shared)

# The programs to build
add_subdirectory(Source/background)
add_subdirectory(Source/convert)
add_subdirectory(Source/env2mod)
add_subdirectory(Source/fds2fed)
add_subdirectory(Source/flush)
add_subdirectory(Source/get_time)
add_subdirectory(Source/getdate)
if (pp_HASH)
    add_subdirectory(Source/hashfile)
endif()
add_subdirectory(Source/makepo)
add_subdirectory(Source/mergepo)
add_subdirectory(Source/set_path)
add_subdirectory(Source/smokediff)
add_subdirectory(Source/smokeview)
add_subdirectory(Source/smokezip)
add_subdirectory(Source/smvq)
add_subdirectory(Source/timep)
add_subdirectory(Source/wind2fds)

# The programs to build on Windows only
if(WIN32)
    add_subdirectory(Source/runbatch)
endif()
