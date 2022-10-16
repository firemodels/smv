cmake_minimum_required(VERSION 3.21)
project(lua LANGUAGES C VERSION 5.2.1)

set(FILES
    "pthread.c"
)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # add_compile_options(-Wno-pointer-sign)
endif ()

add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB)
#TODO: only for windows (not gcc)
add_compile_definitions(HAVE_STRUCT_TIMESPEC)
# add_library(lua_static STATIC ${LUA_LIB_SRCS})
# set_target_properties(lua_static PROPERTIES OUTPUT_NAME "lua")
include_directories(".")
add_library(pthread_static STATIC ${FILES})
set_target_properties(pthread_static PROPERTIES OUTPUT_NAME "pthread")
