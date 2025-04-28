cmake_minimum_required(VERSION 3.17)
project(pthreads LANGUAGES C VERSION 5.2.1)


set(FILES
    "pthread.c"
)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB)
#TODO: only for windows (not gcc)
add_compile_definitions(HAVE_STRUCT_TIMESPEC)
include_directories(".")
add_library(pthread_static STATIC ${FILES})
set_target_properties(pthread_static PROPERTIES OUTPUT_NAME "pthread")
# IntelLLVM warnings
target_compile_options(pthread_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-single-bit-bitfield-constant-conversion>)
target_compile_options(pthread_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-pointer-sign>)
