cmake_minimum_required(VERSION 3.17)
project(lpeg LANGUAGES C VERSION 5.2.1)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)

FetchContent_Declare(lpeg
    URL http://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.0.2.tar.gz
    URL_HASH     MD5=d342571886f1abcb7afe6a83d024d583 # v1.0.2
)
FetchContent_GetProperties(lpeg)
if(NOT lpeg_POPULATED)
    message("Cloning LPEG")
    FetchContent_Populate(lpeg)
endif()

set(FILES
    ${lpeg_SOURCE_DIR}/lpvm.c
    ${lpeg_SOURCE_DIR}/lpcap.c
    ${lpeg_SOURCE_DIR}/lptree.c
    ${lpeg_SOURCE_DIR}/lpcode.c
    ${lpeg_SOURCE_DIR}/lpprint.c
)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lpeg_shared SHARED ${FILES})
set_target_properties(lpeg_shared PROPERTIES OUTPUT_NAME "lpeg")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif ()

find_package(Lua)
if (LUA_FOUND)
    target_link_libraries(lpeg_shared PRIVATE ${LUA_LIBRARIES})
    target_include_directories(lpeg_shared PRIVATE ${LUA_INCLUDE_DIR})
else()
    target_link_libraries(lpeg_shared PRIVATE lua_shared)
endif()

set_target_properties(lpeg_shared PROPERTIES OUTPUT_NAME lpeg)
set_target_properties(lpeg_shared PROPERTIES PREFIX "")
install(
    TARGETS lpeg_shared
    RENAME lpeg
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
