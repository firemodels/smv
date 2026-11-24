cmake_minimum_required(VERSION 3.17)
project(pthreads LANGUAGES C VERSION 5.2.1)


set(FILES
    "pthread.c"
)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

add_library(pthread_static STATIC ${FILES})
target_compile_definitions(pthread_static PUBLIC PTW32_BUILD PTW32_STATIC_LIB)
#TODO: only for windows (not gcc)
add_compile_definitions(HAVE_STRUCT_TIMESPEC)
target_include_directories(pthread_static PUBLIC ".")
set_target_properties(pthread_static PROPERTIES OUTPUT_NAME "pthread")
# IntelLLVM and clang warnings
if ((CMAKE_C_COMPILER_ID STREQUAL Clang) OR (CMAKE_C_COMPILER_ID STREQUAL IntelLLVM))
    target_compile_options(pthread_static PRIVATE -Wno-single-bit-bitfield-constant-conversion)
    target_compile_options(pthread_static PRIVATE -Wno-pointer-sign)
endif()
