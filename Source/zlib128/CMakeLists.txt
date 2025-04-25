cmake_minimum_required(VERSION 3.17)
project(zlib LANGUAGES C VERSION 5.2.1)

set(FILES
    adler32.c compress.c crc32.c gzclose.c gzlib.c gzread.c gzwrite.c
    uncompr.c deflate.c infback.c trees.c
    zutil.c inflate.c inftrees.c inffast.c
)

include_directories(".")
add_library(zlib_static STATIC ${FILES})
set_target_properties(zlib_static PROPERTIES OUTPUT_NAME "z")
target_include_directories(zlib_static PUBLIC .)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
# IntelLLVM warnings
target_compile_options(zlib_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-deprecated-non-prototype>)
target_compile_options(zlib_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-writable-strings>)
target_compile_options(zlib_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-shift-negative-value>)
target_compile_options(zlib_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-deprecated-declarations>)
