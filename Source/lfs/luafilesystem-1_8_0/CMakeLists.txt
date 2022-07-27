cmake_minimum_required(VERSION 3.21)
project(lua LANGUAGES C VERSION 5.2.1)

set(FILES
    "src/lfs.c"
)

include_directories("src")
include_directories("../../lua-5.3.1/src")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lfs_shared SHARED ${FILES})
target_link_libraries(lfs_shared PUBLIC lua_shared)
set_target_properties(lfs_shared PROPERTIES OUTPUT_NAME "lfs")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
endif ()


install(TARGETS lfs_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
