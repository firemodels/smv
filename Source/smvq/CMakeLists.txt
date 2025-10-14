# smvq
add_executable(smvq smvq.c)

target_include_directories(smvq PRIVATE)
target_link_libraries(smvq PRIVATE smv)

find_package(json-c CONFIG)
if (NOT json-c_FOUND)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(json-c json-c)
    endif()
    if (NOT json-c_FOUND)
        FetchContent_Declare(
            json-c-git
            EXCLUDE_FROM_ALL
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/json-c/json-c
            GIT_REPOSITORY https://github.com/json-c/json-c.git
            GIT_TAG        33337523f5cff82e659ecbe94b2fac19c7761a42
            FIND_PACKAGE_ARGS CONFIG NAMES json-c
        )
        FetchContent_MakeAvailable(json-c-git)
        target_link_libraries(smvq PRIVATE json-c)
        target_include_directories(smvq PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/json-c)
    else()
        message("${json-c_INCLUDE_DIRS}")
        target_link_libraries(smvq  PRIVATE ${json-c_LIBRARIES})
        target_include_directories(smvq  PRIVATE ${json-c_INCLUDE_DIRS})
        target_compile_options(smvq PUBLIC ${json-c_CFLAGS_OTHER})
    endif()
else()
    target_link_libraries(smvq PRIVATE json-c::json-c)
endif()
if (WIN32)
    find_package(unofficial-getopt-win32 REQUIRED)
    target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
    # if (BUILD_SHARED_LIBS)
    #     install(IMPORTED_RUNTIME_ARTIFACTS unofficial::getopt-win32::getopt)
    # endif()
endif()
install(TARGETS smvq)
