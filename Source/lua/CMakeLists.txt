cmake_minimum_required(VERSION 3.21)
project(lua LANGUAGES C VERSION 5.2.1)
cmake_policy(SET CMP0135 NEW)
include(FetchContent)

option(LUA_SUPPORT_DL "Support dynamic loading of compiled modules" ON)
option(LUA_BUILD_AS_CXX "Build lua as C++" OFF)
option(LUA_BUILD_BINARY "Build lua binary" OFF)
option(LUA_BUILD_COMPILER "Build luac compiler" OFF)
if(WIN32)
    add_compile_definitions(LUA_BUILD_AS_DLL)
endif()

FetchContent_Declare(lua_shared
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        5d708c3f9cae12820e415d4f89c9eacbe2ab964b # v5.4.4
)
FetchContent_GetProperties(lua_shared)
FetchContent_Populate(lua_shared)

set(LUA_LIB_SRCS
    ${lua_shared_SOURCE_DIR}/lapi.c
    ${lua_shared_SOURCE_DIR}/lauxlib.c
    ${lua_shared_SOURCE_DIR}/lbaselib.c
    ${lua_shared_SOURCE_DIR}/lcode.c
    ${lua_shared_SOURCE_DIR}/lcorolib.c
    ${lua_shared_SOURCE_DIR}/lctype.c
    ${lua_shared_SOURCE_DIR}/ldblib.c
    ${lua_shared_SOURCE_DIR}/ldebug.c
    ${lua_shared_SOURCE_DIR}/ldo.c
    ${lua_shared_SOURCE_DIR}/ldump.c
    ${lua_shared_SOURCE_DIR}/lfunc.c
    ${lua_shared_SOURCE_DIR}/lgc.c
    ${lua_shared_SOURCE_DIR}/linit.c
    ${lua_shared_SOURCE_DIR}/liolib.c
    ${lua_shared_SOURCE_DIR}/llex.c
    ${lua_shared_SOURCE_DIR}/lmathlib.c
    ${lua_shared_SOURCE_DIR}/lmem.c
    ${lua_shared_SOURCE_DIR}/loadlib.c
    ${lua_shared_SOURCE_DIR}/lobject.c
    ${lua_shared_SOURCE_DIR}/lopcodes.c
    ${lua_shared_SOURCE_DIR}/loslib.c
    ${lua_shared_SOURCE_DIR}/lparser.c
    ${lua_shared_SOURCE_DIR}/lstate.c
    ${lua_shared_SOURCE_DIR}/lstring.c
    ${lua_shared_SOURCE_DIR}/lstrlib.c
    ${lua_shared_SOURCE_DIR}/ltable.c
    ${lua_shared_SOURCE_DIR}/ltablib.c
    ${lua_shared_SOURCE_DIR}/ltm.c
    ${lua_shared_SOURCE_DIR}/lundump.c
    ${lua_shared_SOURCE_DIR}/lutf8lib.c
    ${lua_shared_SOURCE_DIR}/lvm.c
    ${lua_shared_SOURCE_DIR}/lzio.c
)

if(LUA_BUILD_AS_CXX)
	set_source_files_properties(${LUA_LIB_SRCS} ${lua_shared_SOURCE_DIR}/lua.c ${lua_shared_SOURCE_DIR}/luac.c PROPERTIES LANGUAGE CXX )
endif()

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lua_shared SHARED ${LUA_LIB_SRCS})
set_target_properties(lua_shared PROPERTIES OUTPUT_NAME "lua")
target_include_directories(lua_shared PUBLIC ${lua_shared_SOURCE_DIR})
if(UNIX)
    if (UNIX AND NOT(EMSCRIPTEN))
        find_library(LIBM m)
        if(NOT LIBM)
            message(FATAL_ERROR "libm not found and is required by lua")
        endif()
        target_link_libraries(lua_shared PUBLIC ${LIBM})
    endif()
endif()

if(LUA_BUILD_BINARY)
    add_executable(lua_exe ${lua_shared_SOURCE_DIR}/lua.c)
    target_link_libraries(lua_exe PUBLIC lua_shared)
    if(NOT(WIN32))
        # Build with readline when not on windows
        pkg_check_modules(Readline REQUIRED IMPORTED_TARGET readline)
        target_link_libraries(lua_exe PRIVATE PkgConfig::Readline)
    endif()
    if (UNIX AND NOT(EMSCRIPTEN))
        target_link_libraries(lua_exe PUBLIC ${LIBM})
    endif()
    install(
        TARGETS lua_exe
        RENAME lua
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(LUA_BUILD_COMPILER)
    add_executable(luac ${lua_shared_SOURCE_DIR}/luac.c)
    target_link_libraries(luac PUBLIC lua_shared)
    set_target_properties(luac PROPERTIES)
endif()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

install(TARGETS lua_shared)
