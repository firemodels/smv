cmake_minimum_required(VERSION 3.21)
project(lpeg LANGUAGES C VERSION 5.2.1)

set(FILES
    "lpvm.c" "lpcap.c" "lptree.c" "lpcode.c" "lpprint.c"
)

include_directories("../lua-5.3.1/src")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lpeg_shared SHARED ${FILES})
target_link_libraries(lpeg_shared PUBLIC lua_shared)
set_target_properties(lpeg_shared PROPERTIES OUTPUT_NAME "lpeg")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif ()


install(TARGETS lpeg_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
