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
    add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB GLEW_STATIC X64 _WIN32 WIN32 _CONSOLE)
endif()
# Set flags for Linux
if (LINUX)
    add_definitions(-Dpp_LINUX)
endif()

# This is a set of warnings that are not enabled by default but it would be good
# to satisfy these.
if(STRICT_CHECKS)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
    add_compile_options(-O2)
    add_compile_options(-flto=auto)
    add_compile_options(-ffat-lto-objects)
    add_compile_options(-fexceptions)
    add_compile_options(-grecord-gcc-switches)
    add_compile_options(-Wall)
    add_compile_options(-Werror=format-security)
    add_compile_options(-Wp,-D_FORTIFY_SOURCE=2)
    add_compile_options(-Wp,-D_GLIBCXX_ASSERTIONS )
    add_compile_options(-fstack-protector-strong)
    add_compile_options(-fasynchronous-unwind-tables)
    add_compile_options(-fstack-clash-protection -fcf-protection)
    add_compile_options(-fcf-protection)
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

add_executable(smokeview
    Source/smokeview/main.c
    Source/smokeview/menus.c
    Source/smokeview/IOscript.c
    Source/smokeview/IOshooter.c
    Source/shared/csphere.c
    Source/smokeview/colortimebar.c
    Source/smokeview/camera.c
    Source/smokeview/IOgeometry.c
    Source/smokeview/IOwui.c
    Source/smokeview/IOobjects.c
    Source/smokeview/IOtour.c
    Source/smokeview/getdatacolors.c
    Source/smokeview/smokeview.c
    Source/smokeview/output.c
    Source/smokeview/renderimage.c
    Source/smokeview/renderhtml.c
    Source/shared/isobox.c
    Source/smokeview/getdatabounds.c
    Source/smokeview/readsmv.c
    Source/smokeview/scontour2d.c
    Source/shared/dmalloc.c
    Source/shared/compress.c
    Source/smokeview/IOvolsmoke.c
    Source/smokeview/IOsmoke.c
    Source/smokeview/IOplot3d.c
    Source/smokeview/IOplot2d.c
    Source/smokeview/IOslice.c
    Source/smokeview/IOhvac.c
    Source/smokeview/IOboundary.c
    Source/smokeview/IOpart.c
    Source/smokeview/IOzone.c
    Source/smokeview/IOiso.c
    Source/smokeview/callbacks.c
    Source/smokeview/drawGeometry.c
    Source/smokeview/skybox.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/smokeview/startup.c
    Source/smokeview/shaders.c
    Source/smokeview/unit.c
    Source/shared/threader.c
    Source/shared/histogram.c
    Source/shared/translate.c
    Source/smokeview/update.c
    Source/smokeview/viewports.c
    Source/smokeview/smv_geometry.c
    Source/smokeview/showscene.c
    Source/smokeview/infoheader.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_m.c
    Source/shared/stdio_buffer.c
    Source/shared/getdata.c
    Source/shared/color2rgb.c
    Source/shared/readimage.c
    Source/smokeview/colortable.c
    Source/smokeview/command_args.c
)

# GLUI sources are separate here. In some branches GLUI is conditional.
target_sources(smokeview PRIVATE
    Source/smokeview/glui_smoke.cpp
    Source/smokeview/glui_clip.cpp
    Source/smokeview/glui_stereo.cpp
    Source/smokeview/glui_geometry.cpp
    Source/smokeview/glui_motion.cpp
    Source/smokeview/glui_bounds.cpp
    Source/smokeview/glui_colorbar.cpp
    Source/smokeview/glui_display.cpp
    Source/smokeview/glui_tour.cpp
    Source/smokeview/glui_trainer.cpp
    Source/smokeview/glui_objects.cpp
    Source/smokeview/glui_shooter.cpp
)

# These include directories are an existing workaround that would be good to
# remove.Because of this custom code and the potential conflict with native libs
# this needs to be early in the include order.
if (WIN32)
    target_include_directories(smokeview PRIVATE Source/glut_gl)
else()
    target_include_directories(smokeview PRIVATE Source/glui_gl)
endif ()
# GLUI can be provided natively, but there are modifications in the code
# vendored with Smokeview that we rely on. Because of this custom code and the
# potential conflict with native libs this needs to be early in the include order.
target_link_libraries(smokeview PRIVATE glui_static)
target_include_directories(smokeview PRIVATE Source/glui_v2_1_beta)
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(smokeview PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(smokeview PRIVATE Source/pthreads)
        target_link_libraries(smokeview PRIVATE pthread_static)
    endif()
endif()
# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smokeview PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
elseif (GLUT_FOUND)
    target_link_libraries(smokeview PRIVATE GLUT::GLUT)
else()
    target_link_libraries(smokeview PRIVATE glut32_static)
endif ()
if (GLEW_FOUND)
    target_link_libraries(smokeview PRIVATE GLEW::GLEW)
    # This line is a hack to work around the fact the code includes "glew.h"
    # rather than <GL/glew.h>
    target_include_directories(smokeview PRIVATE ${GLEW_INCLUDE_DIRS}/GL)
else()
    target_sources(smokeview PRIVATE
        Source/glew/glew.c
    )
    target_include_directories(smokeview PRIVATE Source/glew)
endif()
if (JPEG_FOUND)
    target_link_libraries(smokeview PRIVATE JPEG::JPEG)
else()
    target_link_libraries(smokeview PRIVATE jpeg_static)
endif()
if (PNG_FOUND)
    target_link_libraries(smokeview PRIVATE PNG::PNG)
else()
    target_link_libraries(smokeview PRIVATE png_static)
endif()
if (ZLIB_FOUND)
    target_link_libraries(smokeview PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(smokeview PRIVATE zlib_static)
endif()
if (LIBGD_FOUND)
    target_link_libraries(smokeview PRIVATE PkgConfig::LIBGD)
else()
    target_link_libraries(smokeview PRIVATE gd_static)
endif()
target_link_libraries(smokeview PRIVATE OpenGL::GL OpenGL::GLU)



target_include_directories(smokeview PRIVATE
    Source/smokeview
    Source/shared
)

if (BETA)
    add_compile_definitions(pp_BETA)
endif ()

if (SMOKEVIEW_CONFIG_PATH)
    target_compile_definitions(smokeview PRIVATE SMOKEVIEW_CONFIG_PATH="${SMOKEVIEW_CONFIG_PATH}")
endif()

if (SMOKEVIEW_OBJECT_DEFS_PATH)
    target_compile_definitions(smokeview PRIVATE SMOKEVIEW_OBJECT_DEFS_PATH="${SMOKEVIEW_OBJECT_DEFS_PATH}")
endif()

if (LUA)
    target_compile_definitions(smokeview PRIVATE pp_LUA)
    find_package(Lua)
    if (LUA_FOUND)
        target_link_libraries(smokeview PRIVATE ${LUA_LIBRARIES})
        target_include_directories(smokeview PRIVATE ${LUA_INCLUDE_DIR})
        if (LUA_BUILD_BINARY)
            # If we want to build the lua binary, we can't just rely on the
            # native shared library so we have to pull in and compile lua this
            # way as well.
            add_subdirectory(Source/lua)
        endif()
    else()
        add_subdirectory(Source/lua)
        target_link_libraries(smokeview PRIVATE lua_shared)
    endif()

    # If we are including the lua interpreter, include the appropriate sources.
    target_sources(smokeview PRIVATE
        Source/smokeview/lua_api.c
        Source/smokeview/c_api.c
    )

    target_sources(smokeview PUBLIC
        Source/smvluacore/smv.lua Source/smvluacore/ssf.lua
        Source/smvluacore/ssfparser.lua Source/smvluacore/ssfcommands.lua
        Source/smvluacore/clipping.lua Source/smvluacore/bounds.lua
        Source/smvluacore/render.lua Source/smvluacore/load.lua
        Source/smvluacore/view.lua Source/smvluacore/tour.lua
        Source/smvluacore/iniparser.lua Source/smvluacore/inioptions.lua
        Source/smvluacore/unload.lua Source/smvluacore/constants.lua
        Source/smvluacore/gnuplot.lua Source/smvluacore/plot.lua
        Source/smvluacore/pl3d.lua Source/smvluacore/json.lua
        Source/smvluacore/camera.lua
    )
    install(
        FILES
        Source/smvluacore/smv.lua Source/smvluacore/ssf.lua
        Source/smvluacore/ssfparser.lua Source/smvluacore/ssfcommands.lua
        Source/smvluacore/clipping.lua Source/smvluacore/bounds.lua
        Source/smvluacore/render.lua Source/smvluacore/load.lua
        Source/smvluacore/view.lua Source/smvluacore/tour.lua
        Source/smvluacore/iniparser.lua Source/smvluacore/inioptions.lua
        Source/smvluacore/unload.lua Source/smvluacore/constants.lua
        Source/smvluacore/gnuplot.lua Source/smvluacore/plot.lua
        Source/smvluacore/pl3d.lua Source/smvluacore/json.lua
        Source/smvluacore/camera.lua
        DESTINATION ${CMAKE_INSTALL_DATADIR}/smokeview/scripts
    )
    if (VENDOR_LFS)
        add_subdirectory(Source/lfs)
    endif()
    if (VENDOR_LPEG)
        add_subdirectory(Source/lpeg)
    endif()
endif()

if (MSVC)
    # The headers in Source/glut_gl redefine some macros. Ideally those headers
    # could be removed but for now we just ignore the warning.
    target_compile_options(smokeview PRIVATE /wd4005)
endif()
if (LINUX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smokeview PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()

install(TARGETS smokeview)

# Other programs
# smokediff
add_executable(smokediff
    Source/shared/dmalloc.c
    Source/shared/histogram.c
    Source/smokediff/IOdboundary.c
    Source/smokediff/IOdplot.c
    Source/smokediff/IOdslice.c
    Source/smokediff/main.c
    Source/smokediff/readsmv.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/smokediff/utilities.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
    Source/shared/getdata.c
    Source/shared/color2rgb.c
)
target_include_directories(smokediff PRIVATE
    Source/smokediff
    Source/smokeview
    Source/shared
)

if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(smokediff PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(smokediff PRIVATE Source/pthreads)
        target_link_libraries(smokediff PRIVATE pthread_static)
    endif()
endif()

if (LINUX)
    target_link_libraries(smokediff m)
endif()

install(TARGETS smokediff)

# smokezip
add_executable(smokezip
    Source/smokezip/main.c
    Source/smokezip/CNV3dsmoke.c
    Source/smokezip/CNVboundary.c
    Source/smokezip/CNVpart.c
    Source/smokezip/CNVslice.c
    Source/shared/dmalloc.c
    Source/smokezip/readfiles.c
    Source/smokezip/utilities.c
    Source/shared/isobox.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/smokezip/threaderzip.c
    Source/shared/threader.c
    Source/shared/compress.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
    Source/shared/getdata.c
)

target_include_directories(smokezip PRIVATE
    Source/smokezip
    Source/smokeview
    Source/shared
    Source/zlib128
)

if (ZLIB_FOUND)
    target_link_libraries(smokezip PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(smokezip PRIVATE zlib_static)
endif()

if (LINUX)
    target_link_libraries(smokezip PRIVATE pthread m)
endif()

if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(smokezip PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(smokezip PRIVATE Source/pthreads)
        target_link_libraries(smokezip PRIVATE pthread_static)
    endif()
endif()

install(TARGETS smokezip)


# background
add_executable(background
    Source/background/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(background PRIVATE
    Source/background
    Source/shared
)

if (LINUX)
    target_link_libraries(background m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(background PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(background PRIVATE Source/pthreads)
        target_link_libraries(background PRIVATE pthread_static)
    endif()
endif()

# convert
add_executable(convert
    Source/convert/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(convert PRIVATE
    Source/convert
    Source/shared
)

if (LINUX)
    target_link_libraries(convert m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(convert PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(convert PRIVATE Source/pthreads)
        target_link_libraries(convert PRIVATE pthread_static)
    endif()
endif()

# env2mod
add_executable(env2mod
    Source/env2mod/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
    Source/env2mod/env2mod.c
)

target_include_directories(env2mod PRIVATE
    Source/env2mod
    Source/shared
)

if (LINUX)
    target_link_libraries(env2mod m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(env2mod PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(env2mod PRIVATE Source/pthreads)
        target_link_libraries(env2mod PRIVATE pthread_static)
    endif()
endif()

# flush
add_executable(flush
    Source/flush/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(flush PRIVATE
    Source/flush
    Source/shared
)

if (LINUX)
    target_link_libraries(flush m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(flush PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(flush PRIVATE Source/pthreads)
        target_link_libraries(flush PRIVATE pthread_static)
    endif()
endif()

# get_time
add_executable(get_time
    Source/get_time/get_time.c
)

# getdate
add_executable(getdate
    Source/getdate/main.c
)

target_include_directories(getdate PRIVATE Source/getdate)


# hashfile
add_executable(hashfile
    Source/hashfile/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(hashfile PRIVATE
    Source/hashfile
    Source/shared
)

if (LINUX)
    target_link_libraries(hashfile m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(hashfile PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(hashfile PRIVATE Source/pthreads)
        target_link_libraries(hashfile PRIVATE pthread_static)
    endif()
endif()

# makepo
add_executable(makepo
    Source/makepo/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(makepo PRIVATE
    Source/makepo
    Source/shared
)

if (LINUX)
    target_link_libraries(makepo m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(makepo PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(makepo PRIVATE Source/pthreads)
        target_link_libraries(makepo PRIVATE pthread_static)
    endif()
endif()

# mergepo
add_executable(mergepo
    Source/mergepo/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/translate.c
    Source/shared/stdio_buffer.c
)

target_include_directories(mergepo PRIVATE
    Source/mergepo
    Source/shared
)

if (LINUX)
    target_link_libraries(mergepo m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(mergepo PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(mergepo PRIVATE Source/pthreads)
        target_link_libraries(mergepo PRIVATE pthread_static)
    endif()
endif()

if (WIN32)
    # set_path
    add_executable(set_path
        Source/set_path/main.c
        Source/shared/dmalloc.c
        Source/shared/file_util.c
        Source/shared/string_util.c
        Source/shared/md5.c
        Source/shared/sha1.c
        Source/shared/sha256.c
        Source/shared/stdio_buffer.c
    )

    target_include_directories(set_path PRIVATE
        Source/set_path
        Source/shared
    )
    if(WIN32)
        if (PThreads4W_FOUND)
            target_link_libraries(set_path PRIVATE PThreads4W::PThreads4W)
        else()
            target_include_directories(set_path PRIVATE Source/pthreads)
            target_link_libraries(set_path PRIVATE pthread_static)
        endif()
    endif()
endif ()

# sh2bat
add_executable(sh2bat
    Source/sh2bat/sh2bat.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(sh2bat PRIVATE
    Source/sh2bat
    Source/shared
)

if (LINUX)
    target_link_libraries(sh2bat m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(sh2bat PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(sh2bat PRIVATE Source/pthreads)
        target_link_libraries(sh2bat PRIVATE pthread_static)
    endif()
endif()

# timep
add_executable(timep
    Source/timep/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(timep PRIVATE
    Source/timep
    Source/shared
)

if (LINUX)
    target_link_libraries(timep m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(timep PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(timep PRIVATE Source/pthreads)
        target_link_libraries(timep PRIVATE pthread_static)
    endif()
endif()

# wind2fds
add_executable(wind2fds
    Source/wind2fds/main.c
    Source/shared/dmalloc.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(wind2fds PRIVATE
    Source/wind2fds
    Source/shared
)

if (LINUX)
    target_link_libraries(wind2fds m)
endif()
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(wind2fds PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(wind2fds PRIVATE Source/pthreads)
        target_link_libraries(wind2fds PRIVATE pthread_static)
    endif()
endif()

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

add_subdirectory(Source/smvq)
