cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(smv LANGUAGES C CXX)

set_property(GLOBAL PROPERTY C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

find_package(OpenGL REQUIRED)
# message( "OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
find_package(GLUT)
# if (NOT GLUT_FOUND)
#     #TODO need to handle debug vs release
#     message(STATUS "Local GLUT not found, using vendored freeglut")
#     set(GLUT_INCLUDE_DIR "Source/freeglut-3.2.1/include")
#     set(GLUT_LIBRARIES "Source/freeglut-3.2.1/lib/freeglut_static")
#     set(GLUT_glut_LIBRARY_DEBUG "Source/freeglut-3.2.1/lib/freeglut_staticd")
#     set(GLUT_glut_LIBRARY_RELEASE "Source/freeglut-3.2.1/lib/freeglut_static")
#     # message( "GLUT_LIBRARIES=${GLUT_LIBRARIES}")
# endif ()
# add_subdirectory(Source/freeglut-3.2.1)
add_subdirectory(Source/glui_v2_1_beta)
add_subdirectory(Source/gd-2.0.15)
add_subdirectory(Source/zlib128)
add_subdirectory(Source/jpeg-9b)
add_subdirectory(Source/png-1.6.21)

# option(FREEGLUT_STATIC "Statically link freeglut" on)
# if (FREEGLUT_STATIC)
#     add_definitions(-DFREEGLUT_STATIC)
# endif ()

add_executable(smokeview Source/smokeview/main.c Source/smokeview/menus.c Source/smokeview/IOscript.c Source/smokeview/IOshooter.c Source/smokeview/glui_objects.cpp Source/smokeview/glui_shooter.cpp
    Source/shared/csphere.c Source/smokeview/colortimebar.c Source/shared/smv_endian.c Source/smokeview/camera.c Source/smokeview/IOgeometry.c Source/smokeview/IOwui.c Source/smokeview/IOobjects.c
    Source/smokeview/IOtour.c Source/smokeview/glui_display.cpp Source/smokeview/glui_tour.cpp Source/smokeview/getdatacolors.c Source/smokeview/smokeview.c Source/smokeview/output.c Source/smokeview/renderimage.c
    Source/smokeview/renderhtml.c Source/shared/isobox.c Source/smokeview/getdatabounds.c Source/smokeview/readsmv.c Source/smokeview/scontour2d.c Source/smokeview/glui_smoke.cpp Source/smokeview/glui_clip.cpp
    Source/smokeview/glui_stereo.cpp Source/smokeview/glui_geometry.cpp Source/smokeview/glui_motion.cpp Source/smokeview/glui_bounds.cpp Source/shared/dmalloc.c Source/shared/assert.c
    Source/shared/compress.c Source/smokeview/IOvolsmoke.c Source/smokeview/IOsmoke.c Source/smokeview/IOplot3d.c Source/smokeview/IOslice.c Source/smokeview/IOboundary.c Source/smokeview/IOpart.c Source/smokeview/IOzone.c
    Source/smokeview/IOiso.c Source/smokeview/callbacks.c Source/smokeview/drawGeometry.c Source/smokeview/glui_colorbar.cpp Source/smokeview/skybox.c Source/shared/file_util.c Source/shared/string_util.c
    Source/smokeview/startup.c Source/smokeview/glui_trainer.cpp Source/smokeview/shaders.c Source/smokeview/unit.c Source/smokeview/threader.c Source/shared/histogram.c Source/shared/translate.c Source/smokeview/update.c
    Source/smokeview/viewports.c Source/smokeview/smv_geometry.c Source/smokeview/showscene.c Source/glew/glew.c Source/smokeview/infoheader.c  Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_m.c Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c Source/smokeview/getdata.c Source/smokeview/color2rgb.c
    Source/smokeview/IOplot2d.c
)
# target_include_directories(smokeview PRIVATE ${OPENGL_INCLUDE_DIRS}  )

add_definitions(-Dpp_CMAKE)

target_link_libraries(smokeview
#     ${GLUT_LIBRARIES}
#     # ${GLUT_glut_LIBRARY_DEBUG}
    glui_static
    gd_static
    png_static
    jpeg_static
    zlib_static
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
else()
    target_link_libraries(smokeview glut32_static)
    add_subdirectory(Source/glut-3.7.6)
endif()


target_include_directories(smokeview PRIVATE
    Source/smokeview
    Source/shared
    Source/zlib128
    # The headers below are not for use
    # Source/smokeview/include/glut
    Source/jpeg-9b
    # freeglut-3.2.1/include
    Source/gd-2.0.15
    Source/glew
    Source/glui_v2_1_beta
)

option(BETA "Include beta functionality" off)
if (BETA)
    add_compile_definitions(pp_BETA)
endif ()

option(LUA "Include lua scripting" on)
if (LUA)
    add_definitions(-Dpp_LUA)
    add_subdirectory(Source/lua-5.3.1)
    add_subdirectory(Source/lpeg-1.0.0)
    add_subdirectory(Source/lfs/luafilesystem-1_8_0)
    # If we are including the lua interpreter, include the appropriate sources.
    target_sources(smokeview PRIVATE Source/smokeview/lua_api.c Source/smokeview/c_api.c)
    target_include_directories(smokeview PRIVATE Source/lua-5.3.1/src)
    target_link_libraries(smokeview lua_shared)

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
        Source/smvluacore/camera.lua Source/smvluacore/window.lua
    )
    install(
        # TARGETS smokeview
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
        Source/smvluacore/camera.lua Source/smvluacore/window.lua
        DESTINATION bin
        # RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
        # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )


endif ()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32)
    add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB GLEW_STATIC X64 _WIN32 WIN32 _CONSOLE)
    target_include_directories(smokeview PRIVATE Source/pthreads)
    target_link_libraries(smokeview pthread_static)
    target_link_libraries(smokeview opengl32 glu32 gdi32 winmm comdlg32 hid)
    add_subdirectory(Source/pthreads)
    target_include_directories(smokeview PRIVATE Source/glut_gl )
else()
    target_include_directories(smokeview PRIVATE Source/glui_gl )
endif ()

if ((NOT MACOSX) AND UNIX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smokeview pthread X11 Xmu GLU GL m stdc++)
endif()
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smokeview "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif ()

install(TARGETS smokeview
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# Other programs
# smokediff
add_executable(smokediff Source/shared/assert.c Source/shared/dmalloc.c Source/shared/histogram.c
    Source/smokediff/IOdboundary.c Source/smokediff/IOdplot.c Source/smokediff/IOdslice.c Source/smokediff/main.c
    Source/smokediff/readsmv.c Source/shared/file_util.c Source/shared/string_util.c Source/smokediff/utilities.c
    Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c Source/smokeview/getdata.c Source/smokeview/color2rgb.c
)
target_include_directories(smokediff PRIVATE
    Source/smokediff
    Source/smokeview
    Source/shared
)

if (WIN32)
    target_include_directories(smokediff PRIVATE Source/pthreads)
endif ()

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokediff m)
endif()

install(TARGETS smokediff
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# smokezip
add_executable(smokezip Source/smokezip/main.c Source/smokezip/CNV3dsmoke.c Source/smokezip/CNVboundary.c
    Source/smokezip/CNVpart.c Source/smokezip/CNVplot3d.c Source/smokezip/CNVslice.c Source/shared/assert.c
    Source/shared/csphere.c Source/shared/dmalloc.c Source/smokezip/lightsmoke.c Source/shared/histogram.c
    Source/smokezip/readfiles.c Source/smokezip/utilities.c Source/shared/isobox.c Source/shared/file_util.c
    Source/shared/string_util.c Source/smokezip/threader.c Source/shared/compress.c Source/shared/md5.c
    Source/shared/sha1.c Source/shared/sha256.c Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c Source/smokeview/getdata.c
)

target_include_directories(smokezip PRIVATE
    Source/smokezip
    Source/smokeview
    Source/shared
    Source/zlib128
)

target_link_libraries(smokezip
    zlib_static
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokezip pthread m)
endif()

if (WIN32)
    target_include_directories(smokezip PRIVATE Source/pthreads)
    target_link_libraries(smokezip pthread_static)
endif ()

install(TARGETS smokezip
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# background
add_executable(background Source/background/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(background PRIVATE
    Source/background
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(background m)
endif()

# convert
add_executable(convert Source/convert/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(convert PRIVATE
    Source/convert
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(convert m)
endif()

# env2mod
add_executable(env2mod Source/env2mod/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c Source/env2mod/env2mod.c
)

target_include_directories(env2mod PRIVATE
    Source/env2mod
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(env2mod m)
endif()

# flush
add_executable(flush Source/flush/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(flush PRIVATE
    Source/flush
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(flush m)
endif()

# get_time
add_executable(get_time Source/get_time/get_time.c)

# getdate
add_executable(getdate Source/getdate/main.c)

target_include_directories(getdate PRIVATE Source/getdate)


# hashfile
add_executable(hashfile Source/hashfile/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(hashfile PRIVATE
    Source/hashfile
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(hashfile m)
endif()

# makepo
add_executable(makepo Source/makepo/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(makepo PRIVATE
    Source/makepo
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(makepo m)
endif()

# mergepo
add_executable(mergepo Source/mergepo/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c Source/shared/translate.c
    Source/shared/stdio_buffer.c
)

target_include_directories(mergepo PRIVATE
    Source/mergepo
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(mergepo m)
endif()

if (WIN32)
# set_path
add_executable(set_path Source/set_path/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(set_path PRIVATE
    Source/set_path
    Source/shared
)
endif ()

# sh2bat
add_executable(sh2bat Source/sh2bat/sh2bat.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(sh2bat PRIVATE
    Source/sh2bat
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(sh2bat m)
endif()

# timep
add_executable(timep Source/timep/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(timep PRIVATE
    Source/timep
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(timep m)
endif()

# wind2fds
add_executable(wind2fds Source/wind2fds/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(wind2fds PRIVATE
    Source/wind2fds
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(wind2fds m)
endif()

include(CTest)
enable_testing()
add_subdirectory(Tests)
add_subdirectory(Verification/Visualization)
