cmake_minimum_required(VERSION 3.21)
project(lua LANGUAGES C VERSION 5.2.1)

option(LUA_SUPPORT_DL "Support dynamic loading of compiled modules" ON)
option(LUA_BUILD_AS_CXX "Build lua as C++" OFF)
option(LUA_BUILD_BINARY "Build lua binary" OFF)
option(LUA_BUILD_COMPILER "Build luac compiler" OFF)
if(WIN32)
    add_compile_definitions(LUA_BUILD_AS_DLL)
endif()

set(LUA_LIB_SRCS
    "src/lapi.c"
    "src/lauxlib.c"
    "src/lbaselib.c"
    "src/lbitlib.c"
    "src/lcode.c"
    "src/lcorolib.c"
    "src/lctype.c"
    "src/ldblib.c"
    "src/ldebug.c"
    "src/ldo.c"
    "src/ldump.c"
    "src/lfunc.c"
    "src/lgc.c"
    "src/linit.c"
    "src/liolib.c"
    "src/llex.c"
    "src/lmathlib.c"
    "src/lmem.c"
    "src/loadlib.c"
    "src/lobject.c"
    "src/lopcodes.c"
    "src/loslib.c"
    "src/lparser.c"
    "src/lstate.c"
    "src/lstring.c"
    "src/lstrlib.c"
    "src/ltable.c"
    "src/ltablib.c"
    "src/ltm.c"
    "src/luac.c"
    "src/lundump.c"
    "src/lutf8lib.c"
    "src/lvm.c"
    "src/lzio.c"
)

if(LUA_BUILD_AS_CXX)
	set_source_files_properties(${LUA_LIB_SRCS} "src/lua.c" "src/luac.c" PROPERTIES LANGUAGE CXX )
endif()

# add_library(lua_static STATIC ${LUA_LIB_SRCS})
# set_target_properties(lua_static PROPERTIES OUTPUT_NAME "lua")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(lua_shared SHARED ${LUA_LIB_SRCS})
set_target_properties(lua_shared PROPERTIES OUTPUT_NAME "lua")
if(UNIX)
	set(LUA_DEFINITIONS)

    if(NOT EMSCRIPTEN)
        find_library(LIBM m)
        #TODO: Redo this with find_package
        if(NOT LIBM)
            message(FATAL_ERROR "libm not found and is required by lua")
        endif()
        # target_link_libraries(lua_static INTERFACE ${LIBM})

        list(APPEND LUA_DEFINITIONS LUA_USE_POSIX)
        # if(LUA_SUPPORT_DL)
        #     target_compile_definitions(lua_static PRIVATE "LUA_USE_DLOPEN")
        #     target_link_libraries(lua_static INTERFACE dl)
        # endif()
    endif()

	# target_compile_definitions(lua_static
	# 	PUBLIC ${LUA_DEFINITIONS}
	# )
	# target_compile_options(lua_static
	# 	PRIVATE "-Wall" "-Wextra"
	# )
endif()

if(LUA_BUILD_BINARY)
    include(CheckIncludeFile)
    CHECK_INCLUDE_FILE("readline/readline.h" HAVE_READLINE_READLINE_H)
    add_executable(lua "src/lua.c")
    target_link_libraries(lua PUBLIC lua_shared)
    set_target_properties(lua PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )
    if (HAVE_READLINE_READLINE_H)
        target_compile_definitions(lua PUBLIC "LUA_USE_READLINE")
        target_link_libraries(lua PUBLIC readline)
    endif()
    if (UNIX AND NOT(EMSCRIPTEN))
        target_link_libraries(lua PUBLIC ${LIBM})
    endif()
    install(TARGETS lua
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

endif()
if(LUA_BUILD_COMPILER)
    add_executable(luac "src/luac.c")
    target_link_libraries(luac PUBLIC lua_shared)
    set_target_properties(luac PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )
endif()


if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-string-plus-int")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif ()

install(TARGETS lua_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
