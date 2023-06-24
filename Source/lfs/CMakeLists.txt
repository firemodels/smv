cmake_minimum_required(VERSION 3.17)
project(lua LANGUAGES C VERSION 5.2.1)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)


FetchContent_Declare(lfs
    GIT_REPOSITORY https://github.com/lunarmodules/luafilesystem.git
    GIT_TAG        7c6e1b013caec0602ca4796df3b1d7253a2dd258 # v1.8.0
)
FetchContent_GetProperties(lfs)
if(NOT lfs_POPULATED)
    message("Cloning LFS")
    FetchContent_Populate(lfs)
endif()

set(FILES
    ${lfs_SOURCE_DIR}/src/lfs.c
)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lfs_shared SHARED ${FILES})
set_target_properties(lfs_shared PROPERTIES OUTPUT_NAME "lfs")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
find_package(Lua)
if (LUA_FOUND)
    target_link_libraries(lfs_shared PRIVATE ${LUA_LIBRARIES})
    target_include_directories(lfs_shared PRIVATE ${LUA_INCLUDE_DIR})
else()
    target_link_libraries(lfs_shared PRIVATE lua_shared)
endif()


set_target_properties(lfs_shared PROPERTIES OUTPUT_NAME lfs)
set_target_properties(lfs_shared PROPERTIES PREFIX "")
install(
    TARGETS lfs_shared
    RENAME lfs
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
