# We select CMake version 3.25 as it allows us to use the LINUX variable.
cmake_minimum_required(VERSION 3.24)
# Set a policy to use the newer MSVC ABI selection
cmake_policy(SET CMP0091 NEW)
# Set a policy to use the newer approach to timestamps in FetchContent
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19")
    cmake_policy(SET CMP0110 NEW)
endif()
include(FetchContent)

# Selected the multithreaded runtime when using MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
project(smv LANGUAGES C CXX VERSION 1.17.0)
# Use C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
# Requires adherence to this standard
set(CMAKE_C_STANDARD_REQUIRED TRUE)
if(WIN32)
    # Set the folder for executables and libraries as the install directory.
    # Using the same conventions as  GNUInstallDirs may be better but this
    # matches the current setup.
    set(CMAKE_INSTALL_BINDIR ".")
    set(CMAKE_INSTALL_LIBDIR ".")
    set(CMAKE_INSTALL_SYSCONFDIR ".")
else()
    # GNUInstallDirs is used to set the appropriate paths on linux (/usr/lib64
    # etc).
    include(GNUInstallDirs)
endif()
# Pass a flag to to the source code. There's no great need for this but allows
# us to have conditional code based on the build method.
# add_definitions(-Dpp_CMAKE)
add_compile_definitions(pp_GITHASH="${PROJECT_VERSION}+sc2")


include(InstallRequiredSystemLibraries)
configure_file(LICENSE.md LICENSE.txt)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_GENERATOR "WIX")
set(CPACK_WIX_UPGRADE_GUID "1940d8a1-c842-442f-bf5d-00a570219bdb")
set(CPACK_WIX_INSTALL_SCOPE "perUser")
set(CPACK_PACKAGE_NAME "Smokeview JSONRPC")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_VENDOR "firemodels")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "A modified smokeview with the ability to communicate via JSONRPC.")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Build/for_bundle/icon.ico")
set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cpack_patch.xml")
include(CPack)
message("Package Name: ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.msi")

# Options which can be configured (along with their defaults)
option(BETA "Include beta functionality" OFF)
option(STRICT_CHECKS "Run additional strict checks" OFF)
option(VENDORED_UI_LIBS "Use the vendored GLUT" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Ensure all files are in the right place to run the tests
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Detect and set flags for Mac
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
endif()
if (UNIX AND NOT MACOSX)
    set(LINUX TRUE)
endif()
# Set flags for Windows
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32)
    add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB   X64 _WIN32 WIN32 _CONSOLE)
endif()
# Set flags for Linux
if (LINUX)
    add_definitions(-Dpp_LINUX)
endif()

# add_compile_definitions(pp_CMAKE)

find_package(OpenGL REQUIRED)
if (VENDORED_UI_LIBS)
    if (NOT MACOSX)
        add_subdirectory(Source/glut-3.7.6)
    endif()
    add_subdirectory(Source/glui_v2_1_beta)
else()
    FetchContent_Declare(
        glui-git
        GIT_REPOSITORY https://github.com/JakeOShannessy/glui.git
        GIT_TAG        fb8380b8603b77aea997428af3fba41c70920b2f
    )
endif()
find_package(GLEW REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(json-c CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGD REQUIRED IMPORTED_TARGET gdlib)
if (WIN32)
    find_package(PThreads4W REQUIRED)
endif()

if (NOT MSVC)
    add_compile_options(-Wunused)
else()
    add_compile_options(/W3)
endif()
# This is a set of warnings that are not enabled by default but it would be good
# to satisfy these.
if(STRICT_CHECKS AND (NOT MSVC))
    # set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    # set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
    add_compile_options(-O2)
    add_compile_options(-fexceptions)
    add_compile_options(-grecord-gcc-switches)
    add_compile_options(-Wall)
    add_compile_options(-Werror=format-security)
    add_compile_options(-Wp,-D_FORTIFY_SOURCE=2)
    add_compile_options(-Wp,-D_GLIBCXX_ASSERTIONS )
    add_compile_options(-fstack-protector-strong)
    add_compile_options(-fasynchronous-unwind-tables)
    add_compile_options(-fstack-clash-protection)
    add_compile_options(-fcf-protection)

    add_compile_options(-Wno-unknown-pragmas)
    add_compile_options(-Wno-uninitialized)
    add_compile_options(-Wno-unused-result)
    add_compile_options(-Wno-format-overflow)
    add_compile_options(-Wno-format-truncation)
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:$<BUILD_INTERFACE:-Wno-overloaded-virtual;-Wno-unused-comparison>>")


    add_compile_options(-Wno-nonnull)
    add_compile_options(-Wno-comment)
    if (NOT (CMAKE_C_COMPILER_ID STREQUAL IntelLLVM))
        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-Wno-stringop-truncation)
        endif()
        add_compile_options(-ffat-lto-objects)
        add_compile_options(-flto=auto)
    else()
        # IntelLLVM-specific options
        add_compile_options(-Wno-tautological-constant-compare)
        add_compile_options(-Wno-for-loop-analysis)
    endif()

    # add_compile_options(-Wshadow)
elseif(STRICT_CHECKS AND (NOT MSVC))
    add_compile_options(/Wall)
    add_compile_options(/Werror)
endif()


include(CTest)
enable_testing()
add_subdirectory(Tests)
option(VERIFICATION_TESTS "Run larger verification tests" OFF)
if (VERIFICATION_TESTS)
    add_subdirectory(Verification/Visualization)
endif()

# Libraries to build (which may be dependencies of the programs below)
add_subdirectory(Source/shared)

# The programs to build
add_subdirectory(Source/background)
add_subdirectory(Source/convert)
add_subdirectory(Source/env2mod)
add_subdirectory(Source/fds2fed)
add_subdirectory(Source/flush)
add_subdirectory(Source/get_time)
add_subdirectory(Source/getdate)
if (pp_HASH)
    add_subdirectory(Source/hashfile)
endif()
add_subdirectory(Source/makepo)
add_subdirectory(Source/mergepo)
add_subdirectory(Source/smokediff)
add_subdirectory(Source/smokeview)
add_subdirectory(Source/smokezip)
add_subdirectory(Source/smvq)
add_subdirectory(Source/timep)
add_subdirectory(Source/wind2fds)

# The programs to build on Windows only
if(WIN32)
    add_subdirectory(Source/runbatch)
    add_subdirectory(Source/set_path)
endif()

# Install config files
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Build/for_bundle/smokeview.ini" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/smokeview")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Build/for_bundle/objects.svo" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/smokeview")
