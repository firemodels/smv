cmake_minimum_required(VERSION 3.21)
project(lua LANGUAGES C VERSION 5.2.1)

set(FILES
    adler32.c compress.c crc32.c gzclose.c gzlib.c gzread.c gzwrite.c
    uncompr.c deflate.c infback.c trees.c
    zutil.c inflate.c inftrees.c inffast.c
)

include_directories(".")
add_library(zlib_static STATIC ${FILES})
set_target_properties(zlib_static PROPERTIES OUTPUT_NAME "z")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-shift-negative-value")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif ()
